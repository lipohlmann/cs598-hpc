"""Plot the homework 2 figures from the CSV files the C programs write.

    python3 plot_results.py [results_directory] [figures_directory]

Defaults: results  ../report/figures.  A figure whose CSV file is missing
is skipped with a note.
"""

import glob
import os
import sys

import matplotlib

matplotlib.use("Agg")  # no display needed, so this also runs in a batch job
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.colors import LinearSegmentedColormap

results_directory = sys.argv[1] if len(sys.argv) > 1 else "results"
figures_directory = sys.argv[2] if len(sys.argv) > 2 else "../report/figures"

# One fixed colour per grid size, so a grid size looks the same in every figure.
COLOR_FOR_GRID_SIZE = {
    32: "#2a78d6",
    128: "#eb6834",
    512: "#1baf7a",
    2048: "#eda100",
    8192: "#e87ba4",
}
LINE_STYLE_FOR_TRANSPOSE_MODE = {"allatonce": "-", "pairwise": "--"}
MARKER_FOR_TRANSPOSE_MODE = {"allatonce": "o", "pairwise": "s"}
SIGNED_VALUE_COLORMAP = LinearSegmentedColormap.from_list(
    "blue_gray_red", ["#2a78d6", "#f0efec", "#e34948"]
)

plt.rcParams.update(
    {
        "figure.figsize": (7, 5.5),
        "axes.prop_cycle": plt.cycler(color=["#2a78d6"]),
        "lines.linewidth": 2,
        "lines.markersize": 6,
        "axes.grid": True,
        "grid.color": "#e1e0d9",
        "axes.spines.top": False,
        "axes.spines.right": False,
        "legend.fontsize": 8,
    }
)


# ---------------------------------------------------------------- helpers


def read_csv_if_present(file_name):
    """Return the table in results_directory/file_name, or None if absent."""
    path = os.path.join(results_directory, file_name)
    if not os.path.exists(path):
        print(f"skipping: {path} not found")
        return None
    return pd.read_csv(path)


def read_timing_table():
    """The merged timing.csv, or every per-run timing file stacked."""
    merged_path = os.path.join(results_directory, "timing.csv")
    if os.path.exists(merged_path):
        return pd.read_csv(merged_path)
    per_run_paths = sorted(glob.glob(os.path.join(results_directory, "timing_P*.csv")))
    if not per_run_paths:
        print(f"skipping: no timing files in {results_directory}")
        return None
    return pd.concat([pd.read_csv(path) for path in per_run_paths])


def save_figure(figure_name):
    os.makedirs(figures_directory, exist_ok=True)
    path = os.path.join(figures_directory, figure_name + ".png")
    plt.tight_layout()
    plt.savefig(path, dpi=200)
    plt.close()
    print(f"wrote {path}")


def plot_one_curve_per_grid_size_and_mode(timing_table, x_column, y_column):
    """One line per (grid size, transpose mode): colour = grid size,
    line style and marker = transpose mode."""
    for (grid_size, transpose_mode), run_group in timing_table.groupby(["Nx", "mode"]):
        run_group = run_group.sort_values(x_column)
        plt.plot(
            run_group[x_column],
            run_group[y_column],
            color=COLOR_FOR_GRID_SIZE.get(grid_size, "black"),
            linestyle=LINE_STYLE_FOR_TRANSPOSE_MODE[transpose_mode],
            marker=MARKER_FOR_TRANSPOSE_MODE[transpose_mode],
            label=f"N = {grid_size}, {transpose_mode}",
        )
    add_legend_below_axes()


def add_legend_below_axes():
    """Below the plot, so the legend never covers a curve."""
    plt.legend(ncols=3, loc="upper center", bbox_to_anchor=(0.5, -0.15))


def set_log_log_axes():
    plt.xscale("log", base=2)
    plt.yscale("log")


# ------------------------------------------------------------ verification


def plot_solution_contours():
    solution_paths = sorted(glob.glob(os.path.join(results_directory, "solution_*.csv")))
    if not solution_paths:
        print(f"skipping: no solution files in {results_directory}")
    for solution_path in solution_paths:
        solution_table = pd.read_csv(solution_path)
        computed_solution = solution_table.pivot(index="y", columns="x", values="u")
        solution_error = solution_table.pivot(index="y", columns="x", values="error")
        # The grid size is in the file name: solution_P0004_N512x512_allatonce.csv
        grid_size = os.path.basename(solution_path).split("_N")[1].split("x")[0]

        figure, (solution_axes, error_axes) = plt.subplots(1, 2, figsize=(11, 4.5))
        for axes, values, title in [
            (solution_axes, computed_solution, f"Computed solution, N = {grid_size}"),
            (error_axes, solution_error, "Computed minus exact"),
        ]:
            largest_magnitude = np.abs(values.values).max()
            contour_set = axes.contourf(
                values.columns,
                values.index,
                values.values,
                levels=np.linspace(-largest_magnitude, largest_magnitude, 21),
                cmap=SIGNED_VALUE_COLORMAP,
            )
            figure.colorbar(contour_set, ax=axes)
            axes.set_title(title)
            axes.set_xlabel("x")
            axes.set_ylabel("y")
            axes.set_aspect("equal")
            axes.grid(False)
        save_figure(f"solution_contour_N{grid_size}")


def plot_max_error_versus_grid_size(timing_table):
    error_for_grid_size = timing_table.groupby("Nx")["error"].max()
    grid_sizes = error_for_grid_size.index.to_numpy(dtype=float)
    second_order_reference = error_for_grid_size.iloc[0] * (grid_sizes[0] / grid_sizes) ** 2

    plt.plot(grid_sizes, error_for_grid_size, marker="o", label="max error")
    plt.plot(grid_sizes, second_order_reference, color="#898781", linestyle=":",
             label="second order reference, $N^{-2}$")
    set_log_log_axes()
    plt.xlabel("Grid size N = Nx = Ny")
    plt.ylabel("Max error |u computed - u exact|")
    plt.title("Discretization error versus grid size")
    plt.legend()
    save_figure("max_error_versus_grid_size")


# ----------------------------------------------------------- parallel runs


def plot_total_solve_time_versus_ranks(timing_table):
    plot_one_curve_per_grid_size_and_mode(timing_table, "P", "t_total")
    set_log_log_axes()
    plt.xlabel("Number of ranks P")
    plt.ylabel("Total solve time (s)")
    plt.title("Total solve time versus number of ranks")
    save_figure("total_solve_time_versus_ranks")


def plot_fst_time_versus_ranks(timing_table):
    plot_one_curve_per_grid_size_and_mode(timing_table, "P", "t_fst")
    set_log_log_axes()
    plt.xlabel("Number of ranks P")
    plt.ylabel("Time in local FSTs (s)")
    plt.title("Local FST time versus number of ranks")
    save_figure("fst_time_versus_ranks")


def plot_fst_time_versus_rows_per_rank(timing_table):
    plot_one_curve_per_grid_size_and_mode(timing_table, "M", "t_fst")
    set_log_log_axes()
    plt.xlabel("Rows per rank M")
    plt.ylabel("Time in local FSTs (s)")
    plt.title("Local FST time versus rows per rank")
    save_figure("fst_time_versus_rows_per_rank")


def plot_communication_time_versus_ranks(timing_table):
    # A single rank sends no messages, and zero cannot go on a log axis.
    multi_rank_runs = timing_table[timing_table["P"] > 1]
    plot_one_curve_per_grid_size_and_mode(multi_rank_runs, "P", "t_comm")
    set_log_log_axes()
    plt.xlabel("Number of ranks P")
    plt.ylabel("Time in message calls (s)")
    plt.title("Communication time versus number of ranks")
    save_figure("communication_time_versus_ranks")


def plot_gflops_versus_ranks(timing_table):
    plot_one_curve_per_grid_size_and_mode(timing_table, "P", "gflops")
    set_log_log_axes()
    plt.xlabel("Number of ranks P")
    plt.ylabel("Aggregate GFLOPS")
    plt.title("Aggregate GFLOPS versus number of ranks")
    save_figure("gflops_versus_ranks")


def plot_parallel_efficiency_versus_points_per_rank(timing_table):
    # efficiency = T(1 rank) / (P * T(P ranks)), per grid size and transpose mode
    single_rank_time = (
        timing_table[timing_table["P"] == 1].set_index(["Nx", "mode"])["t_total"]
    )
    efficiency_table = timing_table.join(
        single_rank_time.rename("single_rank_time"), on=["Nx", "mode"]
    )
    efficiency_table["parallel_efficiency"] = efficiency_table["single_rank_time"] / (
        efficiency_table["P"] * efficiency_table["t_total"]
    )
    efficiency_table["grid_points_per_rank"] = (
        efficiency_table["Nx"] * efficiency_table["Ny"] / efficiency_table["P"]
    )

    plot_one_curve_per_grid_size_and_mode(
        efficiency_table, "grid_points_per_rank", "parallel_efficiency"
    )
    plt.axhline(0.8, color="#898781", linestyle=":")
    plt.xscale("log")
    plt.xlabel("Grid points per rank n / P")
    plt.ylabel("Parallel efficiency $T_1 / (P \\, T_P)$")
    plt.title("Parallel efficiency versus grid points per rank (dotted: 80%)")
    save_figure("parallel_efficiency_versus_points_per_rank")


# ----------------------------------------------------- single core, network


def plot_single_core_gflops_versus_grid_points(timing_table):
    single_rank_runs = timing_table[
        (timing_table["P"] == 1) & (timing_table["mode"] == "allatonce")
    ].sort_values("Nx")
    grid_points = single_rank_runs["Nx"] * single_rank_runs["Ny"]

    plt.plot(grid_points, single_rank_runs["gflops"], marker="o")
    plt.xscale("log")
    plt.ylim(bottom=0)
    plt.xlabel("Grid points n = Nx Ny")
    plt.ylabel("GFLOPS on one core")
    plt.title("Single core solver GFLOPS versus grid points")
    save_figure("single_core_gflops_versus_grid_points")


def plot_single_core_fst_gflops_versus_batch_size(fst_table):
    for transform_length, fst_group in fst_table.groupby("n"):
        fst_group = fst_group.sort_values("m")
        plt.plot(
            fst_group["m"],
            fst_group["gflops"],
            marker="o",
            color=COLOR_FOR_GRID_SIZE.get(transform_length + 1, "black"),
            label=f"transform length n = {transform_length}",
        )
    plt.xscale("log", base=2)
    plt.ylim(bottom=0)
    plt.xlabel("Batch size m (rows transformed together)")
    plt.ylabel("GFLOPS on one core")
    plt.title("Single core FST GFLOPS versus batch size")
    plt.legend()
    save_figure("single_core_fst_gflops_versus_batch_size")


def plot_pingpong_time_versus_message_size(pingpong_tables):
    for (placement, pingpong_table), color in zip(
        pingpong_tables.items(), ["#2a78d6", "#eb6834"]
    ):
        plt.plot(
            pingpong_table["nbytes"],
            pingpong_table["t_oneway_min"],
            marker="o",
            color=color,
            label=placement,
        )
    set_log_log_axes()
    plt.xlabel("Message size (bytes)")
    plt.ylabel("One-way message time (s)")
    plt.title("Ping-pong message time versus message size")
    plt.legend()
    save_figure("pingpong_time_versus_message_size")


# ------------------------------------------------------------------- model


def predicted_solve_time(grid_size, num_ranks, fst_table, pingpong_table):
    """Simple model of one solve:

        4 FST passes on a (grid_size / num_ranks) x (grid_size - 1) slab
      + 2 transposes, each (num_ranks - 1) messages of 8 (grid_size / num_ranks)^2 bytes

    It ignores the local transposes and the eigenvalue divide, counts the
    messages as if they were sent one after another, and uses whichever
    ping-pong table it is given for every message.
    """
    rows_per_rank = grid_size / num_ranks

    # measured time of one FST pass: same transform length, nearest batch size
    same_length_rows = fst_table[fst_table["n"] == grid_size - 1]
    nearest_row = same_length_rows.iloc[
        (same_length_rows["m"] - rows_per_rank).abs().argmin()
    ]
    fst_pass_time = nearest_row["t_apply"] * rows_per_rank / nearest_row["m"]

    # measured time of one message, interpolated in log-log
    message_bytes = 8 * rows_per_rank**2
    message_time = np.exp(
        np.interp(
            np.log(message_bytes),
            np.log(pingpong_table["nbytes"]),
            np.log(pingpong_table["t_oneway_min"]),
        )
    )
    return 4 * fst_pass_time + 2 * (num_ranks - 1) * message_time


def plot_model_versus_measured_solve_time(timing_table, fst_table, pingpong_table):
    all_at_once_runs = timing_table[timing_table["mode"] == "allatonce"]
    for grid_size, run_group in all_at_once_runs.groupby("Nx"):
        if grid_size - 1 not in fst_table["n"].values:
            continue
        run_group = run_group.sort_values("P")
        model_times = [
            predicted_solve_time(grid_size, num_ranks, fst_table, pingpong_table)
            for num_ranks in run_group["P"]
        ]
        color = COLOR_FOR_GRID_SIZE.get(grid_size, "black")
        plt.plot(run_group["P"], run_group["t_total"], color=color, marker="o",
                 linestyle="none", label=f"N = {grid_size}, measured")
        plt.plot(run_group["P"], model_times, color=color, linestyle="--",
                 label=f"N = {grid_size}, model")
    set_log_log_axes()
    plt.xlabel("Number of ranks P")
    plt.ylabel("Total solve time (s)")
    plt.title("Model versus measured solve time (all-at-once transpose)")
    add_legend_below_axes()
    save_figure("model_versus_measured_solve_time")


# -------------------------------------------------------------------- main


def main():
    timing_table = read_timing_table()
    fst_table = read_csv_if_present("fst.csv")
    pingpong_tables = {}
    for placement in ["intranode", "internode"]:
        pingpong_table = read_csv_if_present(f"pingpong_{placement}.csv")
        if pingpong_table is not None:
            pingpong_tables[placement] = pingpong_table

    plot_solution_contours()

    if timing_table is not None:
        plot_max_error_versus_grid_size(timing_table)
        plot_total_solve_time_versus_ranks(timing_table)
        plot_fst_time_versus_ranks(timing_table)
        plot_fst_time_versus_rows_per_rank(timing_table)
        plot_communication_time_versus_ranks(timing_table)
        plot_gflops_versus_ranks(timing_table)
        plot_parallel_efficiency_versus_points_per_rank(timing_table)
        plot_single_core_gflops_versus_grid_points(timing_table)

    if fst_table is not None:
        plot_single_core_fst_gflops_versus_batch_size(fst_table)

    if pingpong_tables:
        plot_pingpong_time_versus_message_size(pingpong_tables)

    if timing_table is not None and fst_table is not None and "intranode" in pingpong_tables:
        plot_model_versus_measured_solve_time(
            timing_table, fst_table, pingpong_tables["intranode"]
        )


if __name__ == "__main__":
    main()
