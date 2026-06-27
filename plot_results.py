#!/usr/bin/env python3

from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


RESULTS_PATH = Path("results/simulation_results.csv")
PLOTS_DIR = Path("plots")

COLORS = {
    16: "#1f77b4",
    32: "#d62728",
}

FAMILY_COLORS = {
    "stateless": "#1f77b4",
    "round_robin": "#2ca02c",
    "heap": "#d62728",
}

BIN_MARKERS = {
    16: "o",
    32: "s",
}


def unique_in_order(values):
    seen = set()
    result = []
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return result


def load_results():
    if not RESULTS_PATH.exists():
        raise FileNotFoundError(
            f"{RESULTS_PATH} does not exist. Run `make results` first."
        )

    return pd.read_csv(RESULTS_PATH)


def save_figure(fig, filename):
    PLOTS_DIR.mkdir(exist_ok=True)
    fig.tight_layout()
    fig.savefig(PLOTS_DIR / filename, dpi=200)
    plt.close(fig)


def metric_label(metric):
    if metric == "max_load_mean":
        return "Max load"
    if metric == "cv_load_mean":
        return "CV load"
    return metric


def stddev_column(metric):
    return metric.replace("_mean", "_stddev")


def panel_plot(
    df,
    experiment_id,
    filename,
    metric,
    x_column,
    x_label,
    title,
    reference_column=None,
    reference_label=None,
    title_note=None,
):
    data = df[df["experiment_id"] == experiment_id].copy()
    fig, axes = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
    top_axis, bottom_axis = axes

    categorical = x_column == "policy_name"
    if categorical:
        x_values = unique_in_order(data[x_column].tolist())
        x_lookup = {value: index for index, value in enumerate(x_values)}
    else:
        x_values = sorted(data[x_column].unique())
        x_lookup = {value: value for value in x_values}

    for bins in sorted(data["bins"].unique()):
        subset = data[data["bins"] == bins].copy()
        subset["_x"] = subset[x_column].map(x_lookup)
        subset = subset.sort_values("_x")
        color = COLORS.get(int(bins), None)
        label = f"n={bins}"

        yerr = subset[stddev_column(metric)]
        cost_yerr = subset["cost_per_ball_stddev"]
        top_axis.errorbar(
            subset["_x"],
            subset[metric],
            yerr=yerr,
            marker=BIN_MARKERS.get(int(bins), "o"),
            color=color,
            linewidth=2,
            capsize=3,
            label=label,
        )
        bottom_axis.errorbar(
            subset["_x"],
            subset["cost_per_ball_mean"],
            yerr=cost_yerr,
            marker=BIN_MARKERS.get(int(bins), "o"),
            color=color,
            linewidth=2,
            capsize=3,
            label=label,
        )

        if reference_column is not None:
            top_axis.plot(
                subset["_x"],
                subset[reference_column],
                linestyle="--",
                marker="x",
                color=color,
                alpha=0.8,
                label=f"{reference_label}, n={bins}",
            )

    full_title = title if title_note is None else f"{title}\n{title_note}"
    top_axis.set_title(full_title, fontsize=13)
    top_axis.set_ylabel(metric_label(metric), fontsize=11)
    bottom_axis.set_ylabel("Cost per ball", fontsize=11)
    bottom_axis.set_xlabel(x_label, fontsize=11)
    top_axis.grid(True, alpha=0.25)
    bottom_axis.grid(True, alpha=0.25)
    top_axis.legend(fontsize=9)
    bottom_axis.legend(fontsize=9)

    if categorical:
        bottom_axis.set_xticks(list(range(len(x_values))))
        bottom_axis.set_xticklabels(x_values, rotation=30, ha="right")
    else:
        bottom_axis.set_xticks(x_values)

    save_figure(fig, filename)


def scatter_plot(
    df,
    experiment_id,
    filename,
    metric,
    title,
    reference_column=None,
    reference_note=None,
):
    data = df[df["experiment_id"] == experiment_id].copy()
    fig, axis = plt.subplots(figsize=(10, 7))

    for _, row in data.iterrows():
        family = row["policy_family"]
        bins = int(row["bins"])
        axis.scatter(
            row["cost_per_ball_mean"],
            row[metric],
            color=FAMILY_COLORS.get(family, "#333333"),
            marker=BIN_MARKERS.get(bins, "o"),
            s=70,
            edgecolor="black",
            linewidth=0.5,
        )
        axis.annotate(
            row["policy_name"],
            (row["cost_per_ball_mean"], row[metric]),
            textcoords="offset points",
            xytext=(5, 5),
            fontsize=8,
        )

    family_handles = [
        plt.Line2D(
            [0],
            [0],
            marker="o",
            color="w",
            markerfacecolor=color,
            markeredgecolor="black",
            label=family,
            markersize=8,
        )
        for family, color in FAMILY_COLORS.items()
        if family in set(data["policy_family"])
    ]
    bin_handles = [
        plt.Line2D(
            [0],
            [0],
            marker=marker,
            color="black",
            linestyle="",
            label=f"n={bins}",
            markersize=8,
        )
        for bins, marker in BIN_MARKERS.items()
        if bins in set(data["bins"])
    ]

    reference_handles = []
    if reference_column is not None:
        for bins in sorted(data["bins"].unique()):
            ref_value = data[data["bins"] == bins][reference_column].iloc[0]
            line = axis.axhline(
                ref_value,
                linestyle="--" if int(bins) == 16 else ":",
                color=COLORS.get(int(bins), "gray"),
                linewidth=1.7,
                label=f"reference n={bins}",
            )
            reference_handles.append(line)

    full_title = title if reference_note is None else f"{title}\n{reference_note}"
    axis.set_title(full_title, fontsize=13)
    axis.set_xlabel("Cost per ball", fontsize=11)
    axis.set_ylabel(metric_label(metric), fontsize=11)
    axis.grid(True, alpha=0.25)
    axis.legend(
        handles=family_handles + bin_handles + reference_handles,
        fontsize=9,
        loc="best",
    )

    save_figure(fig, filename)


def main():
    df = load_results()

    panel_plot(
        df,
        "01_stateless_baseline",
        "01_stateless_baseline_max_load.png",
        "max_load_mean",
        "policy_name",
        "Policy",
        "Experiment 1: Stateless baseline - max load",
        reference_column="reference_max_load",
        reference_label="optimal",
    )
    panel_plot(
        df,
        "01_stateless_baseline",
        "01_stateless_baseline_cv_load.png",
        "cv_load_mean",
        "policy_name",
        "Policy",
        "Experiment 1: Stateless baseline - CV load",
        reference_column="reference_cv_load",
        reference_label="optimal CV",
    )
    panel_plot(
        df,
        "02_diminishing_returns",
        "02_diminishing_returns_max_load.png",
        "max_load_mean",
        "k",
        "k",
        "Experiment 2: Diminishing returns of k - max load",
        reference_column="reference_max_load",
        reference_label="optimal",
    )
    panel_plot(
        df,
        "02_diminishing_returns",
        "02_diminishing_returns_cv_load.png",
        "cv_load_mean",
        "k",
        "k",
        "Experiment 2: Diminishing returns of k - CV load",
        reference_column="reference_cv_load",
        reference_label="optimal CV",
    )
    panel_plot(
        df,
        "03_stateful_vs_stateless_unweighted",
        "03_stateful_vs_stateless_unweighted_max_load.png",
        "max_load_mean",
        "policy_name",
        "Policy",
        "Experiment 3: Stateful vs stateless - max load",
        reference_column="reference_max_load",
        reference_label="optimal",
    )
    panel_plot(
        df,
        "03_stateful_vs_stateless_unweighted",
        "03_stateful_vs_stateless_unweighted_cv_load.png",
        "cv_load_mean",
        "policy_name",
        "Policy",
        "Experiment 3: Stateful vs stateless - CV load",
        reference_column="reference_cv_load",
        reference_label="optimal CV",
    )
    panel_plot(
        df,
        "04_weighted_round_robin_break",
        "04_weighted_round_robin_break_max_load.png",
        "max_load_mean",
        "policy_name",
        "Policy",
        "Experiment 4: Weighted balls break round-robin - max load",
        reference_column="reference_max_load",
        reference_label="average-load lower bound",
        title_note="Weighted reference is average-load lower bound, not true optimum",
    )
    panel_plot(
        df,
        "04_weighted_round_robin_break",
        "04_weighted_round_robin_break_cv_load.png",
        "cv_load_mean",
        "policy_name",
        "Policy",
        "Experiment 4: Weighted balls break round-robin - CV load",
    )
    panel_plot(
        df,
        "05_heap_sweep",
        "05_heap_sweep_max_load.png",
        "max_load_mean",
        "heap_size",
        "Heap size s",
        "Experiment 5: Heap size sweep - max load",
        reference_column="reference_max_load",
        reference_label="average-load lower bound",
    )
    panel_plot(
        df,
        "05_heap_sweep",
        "05_heap_sweep_cv_load.png",
        "cv_load_mean",
        "heap_size",
        "Heap size s",
        "Experiment 5: Heap size sweep - CV load",
    )

    scatter_plot(
        df,
        "06_final_tradeoff_unweighted",
        "06_final_tradeoff_unweighted_max_load.png",
        "max_load_mean",
        "Experiment 6: Final tradeoff unweighted - max load",
        reference_column="reference_max_load",
    )
    scatter_plot(
        df,
        "06_final_tradeoff_unweighted",
        "06_final_tradeoff_unweighted_cv_load.png",
        "cv_load_mean",
        "Experiment 6: Final tradeoff unweighted - CV load",
        reference_column="reference_cv_load",
    )
    scatter_plot(
        df,
        "07_final_tradeoff_weighted",
        "07_final_tradeoff_weighted_max_load.png",
        "max_load_mean",
        "Experiment 7: Final tradeoff weighted - max load",
        reference_column="reference_max_load",
        reference_note=(
            "Weighted lower bound = total weight / bins; true optimum requires "
            "partition/bin-packing"
        ),
    )
    scatter_plot(
        df,
        "07_final_tradeoff_weighted",
        "07_final_tradeoff_weighted_cv_load.png",
        "cv_load_mean",
        "Experiment 7: Final tradeoff weighted - CV load",
    )

    print(f"Generated 14 plots in {PLOTS_DIR}/")


if __name__ == "__main__":
    main()
