import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm, colors


def mms(x, y, z):
    return np.sin(4.0*np.pi*x)*np.sin(2.0*np.pi*y)*np.sin(3.0*np.pi*z)


n = 500
s = np.linspace(0.0, 1.0, n)
A, B = np.meshgrid(s, s)

# Slice positions (avoid planes where f vanishes identically, e.g. x = 0.5)
x0, y0, z0 = 0.375, 0.25, 1.0/6.0

slices = [
    (f"$z = {z0:.2f}$", "x", "y", mms(A, B, z0)),
    (f"$y = {y0:.2f}$", "x", "z", mms(A, y0, B)),
    (f"$x = {x0:.3f}$", "y", "z", mms(x0, A, B)),
]

norm = colors.Normalize(vmin=-1, vmax=1)
cmap = cm.magma

fig, axes = plt.subplots(1, 3, figsize=(12, 4.2), sharey=True,
                         constrained_layout=True)
for ax, (title, xl, yl, v) in zip(axes, slices):
    im = ax.pcolormesh(A, B, v, cmap=cmap, norm=norm, shading="auto",
                       rasterized=True)
    ax.set_title(title)
    ax.set_xlabel(f"${xl}$")
    ax.set_ylabel(f"${yl}$")
    ax.set_aspect("equal")
fig.colorbar(im, ax=axes, shrink=0.85, label="$u(x,y,z)$")
fig.suptitle("Manufactured solution: planar slices")
fig.savefig("mms.png", dpi=300)
