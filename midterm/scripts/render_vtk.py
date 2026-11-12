#!/usr/bin/env python3
"""Render the solution written by `midterm --vtk PREFIX` to PNG images.

Usage:
    python scripts/render_vtk.py [PVTU] [--out DIR] [--styles S [S ...]]

    PVTU      the .pvtu (or a single .vtu) file   (default: results/vtk/finest.pvtu)
    --out     output directory                    (default: results/figures)
    --styles  which images to make                (default: all of them)
    --field   point field shown by the solution styles (default: u)

Each style writes DIR/<prefix>_<style>.png:

    slices       three orthogonal slices through an extremum of the field
    stack        a stack of parallel slices along one axis (--axis)
    isosurfaces  nested positive/negative isosurfaces (opaque core, clear shell)
    cutaway      the solid domain with one octant removed
    error        |u - u_exact| on three orthogonal slices, log colour scale
    ranks        the domain decomposition: one block per MPI rank, pulled apart

Requires pyvista (pip install pyvista). Rendering is off-screen, so it also
works over ssh. The finest h-convergence run (13.8 M points, ASCII) takes
about a minute to load and needs about 4 GB of memory.
"""

import argparse
import sys
import time
from pathlib import Path

import numpy as np
import pyvista as pv

STYLES = ["slices", "stack", "isosurfaces", "cutaway", "error", "ranks"]
DIVERGING = "RdBu_r"  # the solution is signed: white = 0
BAR = dict(vertical=True, position_x=0.86, position_y=0.2, height=0.6, width=0.05,
           title_font_size=22, label_font_size=18, fmt="%.2g")


def log(msg):
    print(f"[{time.strftime('%H:%M:%S')}] {msg}", flush=True)


def load(path: Path) -> pv.UnstructuredGrid:
    """Read the mesh and merge the nodes that neighbouring elements duplicate.

    The writer stores every element with its own copy of the GLL nodes. Merging
    them makes the mesh connected, so surfaces are the true outer (or rank)
    boundaries instead of every element face, and isosurfaces are continuous.
    """
    log(f"reading {path}")
    mesh = pv.read(path)
    log(f"  {mesh.n_points:,} points, {mesh.n_cells:,} cells; merging shared nodes")
    mesh = mesh.clean(tolerance=0.0, produce_merge_map=False)
    log(f"  {mesh.n_points:,} unique points")
    return mesh


def surface_of(grid):
    """Outer surface of a grid (extract_surface, across pyvista versions)."""
    try:
        return grid.extract_surface(algorithm="dataset_surface")
    except TypeError:  # older pyvista: no algorithm keyword
        return grid.extract_surface()


def central_extremum(mesh, values):
    """Position of the largest |value|; among ties, the one nearest the centre.

    Slicing through an extremum avoids the nodal planes of the solution, where
    a slice would just show zero.
    """
    mag = np.abs(values)
    candidates = np.flatnonzero(mag >= 0.99 * mag.max())
    pts = mesh.points[candidates]
    nearest = np.argmin(np.linalg.norm(pts - np.array(mesh.center), axis=1))
    return pts[nearest]


def new_plotter(args):
    pl = pv.Plotter(off_screen=True, window_size=args.window_size)
    pl.set_background("white")
    return pl


def finish(pl, mesh, args, style, title):
    pl.add_mesh(mesh.outline(), color="black", line_width=1.5)
    pl.add_axes(color="black", line_width=3)
    pl.add_text(title, font_size=14, color="black", position="upper_left")
    pl.camera_position = "iso"
    pl.camera.azimuth = args.azimuth
    pl.camera.elevation = args.elevation
    pl.reset_camera()
    pl.camera.zoom(0.92)  # margin, so the box corners stay inside the frame
    out = args.out / f"{args.prefix}_{style}.png"
    pl.screenshot(out)
    pl.close()
    log(f"  wrote {out}")


def symmetric_clim(mesh, field):
    vmax = float(np.abs(mesh.point_data[field]).max())
    return [-vmax, vmax]


def render_slices(mesh, args):
    field = args.field
    origin = central_extremum(mesh, mesh.point_data[field])
    slices = mesh.slice_orthogonal(x=origin[0], y=origin[1], z=origin[2])
    pl = new_plotter(args)
    pl.add_mesh(slices, scalars=field, cmap=DIVERGING, clim=symmetric_clim(mesh, field),
                lighting=False, scalar_bar_args=dict(BAR, title=field, color="black"))
    finish(pl, mesh, args, "slices",
           f"{field}: slices through ({origin[0]:.3g}, {origin[1]:.3g}, {origin[2]:.3g})")


def render_stack(mesh, args):
    field = args.field
    axis = "xyz".index(args.axis)
    lo, hi = mesh.bounds[2 * axis], mesh.bounds[2 * axis + 1]
    # midpoints of n equal slabs: evenly spaced, and away from the domain
    # boundary and the mid-plane, where the solution tends to be zero
    positions = lo + (np.arange(args.n_slices) + 0.5) * (hi - lo) / args.n_slices
    normal = np.eye(3)[axis]
    pl = new_plotter(args)
    for i, pos in enumerate(positions):
        origin = np.array(mesh.center)
        origin[axis] = pos
        pl.add_mesh(mesh.slice(normal=normal, origin=origin), scalars=field,
                    cmap=DIVERGING, clim=symmetric_clim(mesh, field),
                    lighting=False, show_scalar_bar=(i == 0),
                    scalar_bar_args=dict(BAR, title=field, color="black"))
    finish(pl, mesh, args, "stack", f"{field}: {args.n_slices} slices along {args.axis}")


def render_isosurfaces(mesh, args):
    field = args.field
    vmax = symmetric_clim(mesh, field)[1]
    pl = new_plotter(args)
    pl.enable_depth_peeling(number_of_peels=8)  # correct blending of the shells
    # an opaque core inside a translucent shell, for both signs
    for i, (fraction, opacity) in enumerate([(0.75, 1.0), (0.3, 0.3)]):
        surface = mesh.contour([-fraction * vmax, fraction * vmax], scalars=field)
        pl.add_mesh(surface, scalars=field, cmap=DIVERGING, clim=[-vmax, vmax],
                    opacity=opacity, smooth_shading=True, show_scalar_bar=(i == 0),
                    scalar_bar_args=dict(BAR, title=field, color="black"))
    finish(pl, mesh, args, "isosurfaces",
           f"{field}: isosurfaces at 0.3 and 0.75 of the peak value, both signs")


def render_cutaway(mesh, args):
    field = args.field
    corner = central_extremum(mesh, mesh.point_data[field])
    # drop the cells of the octant facing the camera; the cut faces then pass
    # through an extremum of the field
    centers = mesh.cell_centers().points
    keep = ~np.all(centers > corner, axis=1)
    solid = surface_of(mesh.extract_cells(np.flatnonzero(keep)))
    pl = new_plotter(args)
    pl.add_mesh(solid, scalars=field, cmap=DIVERGING, clim=symmetric_clim(mesh, field),
                lighting=False, scalar_bar_args=dict(BAR, title=field, color="black"))
    # the outer faces are the (zero) boundary values: draw the edges of the
    # solid so that its shape reads without shading
    pl.add_mesh(solid.extract_feature_edges(feature_angle=30.0), color="black",
                line_width=1.5)
    finish(pl, mesh, args, "cutaway", f"{field}: cutaway")


def render_error(mesh, args):
    if "error" in mesh.point_data:
        err = np.abs(mesh.point_data["error"])
    elif "u" in mesh.point_data and "u_exact" in mesh.point_data:
        err = np.abs(mesh.point_data["u"] - mesh.point_data["u_exact"])
    else:
        log("  no error or u_exact field, skipping the error style")
        return
    mesh.point_data["abs_error"] = err
    origin = central_extremum(mesh, err)
    slices = mesh.slice_orthogonal(x=origin[0], y=origin[1], z=origin[2])
    emax = float(err.max())
    pl = new_plotter(args)
    pl.add_mesh(slices, scalars="abs_error", cmap="viridis", log_scale=True,
                clim=[emax * 1e-3, emax], lighting=False,
                scalar_bar_args=dict(BAR, title="error", color="black"))
    finish(pl, mesh, args, "error", f"error magnitude, u - u_exact (max {emax:.2e})")


def render_ranks(mesh, args):
    if "rank" not in mesh.cell_data:
        log("  no rank field, skipping the ranks style")
        return
    ranks = np.asarray(mesh.cell_data["rank"])
    ids = np.unique(ranks)
    center = np.array(mesh.center)
    pl = new_plotter(args)
    blocks = pv.MultiBlock()
    for r in ids:
        block = surface_of(mesh.extract_cells(np.flatnonzero(ranks == r)))
        # pull every block away from the domain centre
        block.translate(args.explode * (np.array(block.center) - center), inplace=True)
        blocks.append(block)
    exploded = blocks.combine()
    pl.add_mesh(exploded, scalars="rank", cmap="turbo", n_colors=len(ids),
                clim=[ids.min() - 0.5, ids.max() + 0.5], ambient=0.35,
                show_scalar_bar=False)
    finish(pl, exploded, args, "ranks",
           f"domain decomposition: {len(ids)} MPI ranks, one colour each")


RENDERERS = {"slices": render_slices, "stack": render_stack,
             "isosurfaces": render_isosurfaces, "cutaway": render_cutaway,
             "error": render_error, "ranks": render_ranks}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("pvtu", nargs="?", type=Path, default=Path("results/vtk/finest.pvtu"))
    ap.add_argument("--out", type=Path, default=Path("results/figures"))
    ap.add_argument("--prefix", default="solution", help="output file name prefix")
    ap.add_argument("--styles", nargs="+", choices=STYLES, default=STYLES)
    ap.add_argument("--field", default="u", help="point field to show (u, u_exact, error)")
    ap.add_argument("--axis", choices="xyz", default="x", help="axis of the slice stack")
    ap.add_argument("--n-slices", type=int, default=8, help="slices in the stack")
    ap.add_argument("--explode", type=float, default=0.25,
                    help="how far the rank blocks are pulled apart (0 = not at all)")
    ap.add_argument("--azimuth", type=float, default=0.0, help="camera rotation [deg]")
    ap.add_argument("--elevation", type=float, default=0.0, help="camera tilt [deg]")
    ap.add_argument("--window-size", type=int, nargs=2, default=[1600, 1200],
                    metavar=("W", "H"))
    args = ap.parse_args()

    if not args.pvtu.exists():
        sys.exit(f"error: {args.pvtu} not found (run the solver with --vtk PREFIX)")
    args.out.mkdir(parents=True, exist_ok=True)

    mesh = load(args.pvtu)
    if args.field not in mesh.point_data:
        sys.exit(f"error: no point field '{args.field}' "
                 f"(available: {', '.join(mesh.point_data.keys())})")
    for style in args.styles:
        log(f"rendering {style}")
        RENDERERS[style](mesh, args)


if __name__ == "__main__":
    main()
