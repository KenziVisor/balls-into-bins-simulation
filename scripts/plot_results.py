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

FAMILY_MARKERS = {
    "stateless": "o",
    "round_robin": "s",
    "heap": "^",
}

IMPORTANT_POLICIES = {
    "Random",
    "Random weighted",
    "Power-2",
    "Power-2 weighted",
    "Round-robin",
    "Absolute minimum",
    "Absolute minimum weighted",
    "Full heap",
    "Heap s=8",
}

SCENARIO_MODES = [
    "unweighted_empty",
    "weighted_empty",
    "unweighted_initialized",
    "weighted_initialized",
]


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


def remove_old_heap_sweep_plots():
    for filename in ("05_heap_sweep_max_load.png", "05_heap_sweep_cv_load.png"):
        path = PLOTS_DIR / filename
        if path.exists():
            path.unlink()


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


def heap_sweep_plot(df, bins, filename):
    data = df[(df["experiment_id"] == "05_heap_sweep") & (df["bins"] == bins)].copy()
    data = data.sort_values("heap_size")

    fig, axes = plt.subplots(2, 2, figsize=(12, 8), sharex=True)
    max_axis = axes[0][0]
    max_cost_axis = axes[1][0]
    cv_axis = axes[0][1]
    cv_cost_axis = axes[1][1]

    x_values = data["heap_size"]
    x_labels = [
        f"{int(row.heap_size)}\nfull" if int(row.heap_size) == bins else str(int(row.heap_size))
        for row in data.itertuples()
    ]

    max_axis.errorbar(
        x_values,
        data["max_load_mean"],
        yerr=data["max_load_stddev"],
        marker="o",
        color="#1f77b4",
        linewidth=2,
        capsize=3,
        label="Heap policy",
    )
    max_axis.plot(
        x_values,
        data["reference_max_load"],
        linestyle="--",
        marker="x",
        color="#555555",
        linewidth=1.5,
        label="average-load lower bound",
    )
    max_cost_axis.errorbar(
        x_values,
        data["cost_per_ball_mean"],
        yerr=data["cost_per_ball_stddev"],
        marker="o",
        color="#1f77b4",
        linewidth=2,
        capsize=3,
    )

    cv_axis.errorbar(
        x_values,
        data["cv_load_mean"],
        yerr=data["cv_load_stddev"],
        marker="s",
        color="#d62728",
        linewidth=2,
        capsize=3,
        label="Heap policy",
    )
    cv_cost_axis.errorbar(
        x_values,
        data["cost_per_ball_mean"],
        yerr=data["cost_per_ball_stddev"],
        marker="s",
        color="#d62728",
        linewidth=2,
        capsize=3,
    )

    fig.suptitle(
        f"Heap sweep, n={bins}, weighted balls, k=2 for partial heap",
        fontsize=14,
    )
    max_axis.set_title("Max load")
    cv_axis.set_title("CV load")
    max_cost_axis.set_title("Cost for max-load view")
    cv_cost_axis.set_title("Cost for CV view")

    max_axis.set_ylabel("Max load")
    cv_axis.set_ylabel("CV load")
    max_cost_axis.set_ylabel("Cost per ball")
    cv_cost_axis.set_ylabel("Cost per ball")
    max_cost_axis.set_xlabel("Heap size s")
    cv_cost_axis.set_xlabel("Heap size s")

    for axis in (max_axis, max_cost_axis, cv_axis, cv_cost_axis):
        axis.grid(True, alpha=0.25)
        axis.set_xticks(list(x_values))
        axis.set_xticklabels(x_labels)

    max_axis.legend(fontsize=9)
    cv_axis.legend(fontsize=9)
    save_figure(fig, filename)


def pareto_points(data, metric):
    sorted_data = data.sort_values(["cost_per_ball_mean", metric])
    efficient = []
    best_quality = None
    for _, row in sorted_data.iterrows():
        quality = row[metric]
        if best_quality is None or quality < best_quality:
            efficient.append(row)
            best_quality = quality
    return pd.DataFrame(efficient)


def final_tradeoff_plot(
    df,
    experiment_id,
    filename,
    metric,
    title,
    reference_column=None,
    reference_note=None,
):
    data = df[df["experiment_id"] == experiment_id].copy()
    fig, axes = plt.subplots(1, 2, figsize=(14, 6), sharey=False)

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

    for axis, bins in zip(axes, sorted(data["bins"].unique())):
        subset = data[data["bins"] == bins].copy()
        for _, row in subset.iterrows():
            family = row["policy_family"]
            important = row["policy_name"] in IMPORTANT_POLICIES
            axis.scatter(
                row["cost_per_ball_mean"],
                row[metric],
                color=FAMILY_COLORS.get(family, "#333333"),
                marker=FAMILY_MARKERS.get(family, "o"),
                s=95 if important else 48,
                edgecolor="black",
                linewidth=0.7 if important else 0.4,
            )
            if important:
                axis.annotate(
                    row["policy_name"].replace(" weighted", ""),
                    (row["cost_per_ball_mean"], row[metric]),
                    textcoords="offset points",
                    xytext=(5, 5),
                    fontsize=8,
                )

        frontier = pareto_points(subset, metric)
        if len(frontier) > 1:
            axis.plot(
                frontier["cost_per_ball_mean"],
                frontier[metric],
                linestyle="--",
                color="#777777",
                linewidth=1,
                alpha=0.65,
                label="Pareto guide",
            )

        reference_handles = []
        if reference_column is not None:
            ref_value = subset[reference_column].iloc[0]
            line = axis.axhline(
                ref_value,
                linestyle=":",
                color="#444444",
                linewidth=1.6,
                label="reference",
            )
            reference_handles.append(line)

        axis.set_title(f"n={bins}", fontsize=12)
        axis.set_xlabel("Cost per ball", fontsize=11)
        axis.set_ylabel(metric_label(metric), fontsize=11)
        axis.grid(True, alpha=0.25)

        handles = family_handles + reference_handles
        axis.legend(handles=handles, fontsize=9, loc="best")

    full_title = title if reference_note is None else f"{title}\n{reference_note}"
    fig.suptitle(full_title, fontsize=14)

    save_figure(fig, filename)


def main():
    df = load_results()
    remove_old_heap_sweep_plots()

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
    heap_sweep_plot(df, 16, "05_heap_sweep_n16.png")
    heap_sweep_plot(df, 32, "05_heap_sweep_n32.png")

    final_tradeoff_plot(
        df,
        "06_final_tradeoff_unweighted",
        "06_final_tradeoff_unweighted_max_load.png",
        "max_load_mean",
        "Experiment 6: Final tradeoff unweighted - max load",
        reference_column="reference_max_load",
    )
    final_tradeoff_plot(
        df,
        "06_final_tradeoff_unweighted",
        "06_final_tradeoff_unweighted_cv_load.png",
        "cv_load_mean",
        "Experiment 6: Final tradeoff unweighted - CV load",
        reference_column="reference_cv_load",
    )
    final_tradeoff_plot(
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
    final_tradeoff_plot(
        df,
        "07_final_tradeoff_weighted",
        "07_final_tradeoff_weighted_cv_load.png",
        "cv_load_mean",
        "Experiment 7: Final tradeoff weighted - CV load",
    )

    generated_plot_count = 14

    for mode in SCENARIO_MODES:
        main_id = f"08_main_comparison_{mode}"
        tradeoff_id = f"09_tradeoff_{mode}"
        title_mode = mode.replace("_", " ")

        panel_plot(
            df,
            main_id,
            f"08_main_comparison_{mode}_max_load.png",
            "max_load_mean",
            "policy_name",
            "Policy",
            f"Main comparison: {title_mode} - max load",
            reference_column="reference_max_load",
            reference_label=(
                "optimal" if mode == "unweighted_empty" else "average-load lower bound"
            ),
        )
        panel_plot(
            df,
            main_id,
            f"08_main_comparison_{mode}_cv_load.png",
            "cv_load_mean",
            "policy_name",
            "Policy",
            f"Main comparison: {title_mode} - CV load",
            reference_column="reference_cv_load" if mode == "unweighted_empty" else None,
            reference_label="optimal CV",
        )
        final_tradeoff_plot(
            df,
            tradeoff_id,
            f"09_tradeoff_{mode}_max_load.png",
            "max_load_mean",
            f"Cost-quality tradeoff: {title_mode} - max load",
            reference_column="reference_max_load",
            reference_note=(
                "Lower bound = total load / bins; true optimum is not computed"
                if mode != "unweighted_empty"
                else None
            ),
        )
        final_tradeoff_plot(
            df,
            tradeoff_id,
            f"09_tradeoff_{mode}_cv_load.png",
            "cv_load_mean",
            f"Cost-quality tradeoff: {title_mode} - CV load",
            reference_column="reference_cv_load" if mode == "unweighted_empty" else None,
        )
        generated_plot_count += 4

    print(f"Generated {generated_plot_count} plots in {PLOTS_DIR}/")


if __name__ == "__main__":
    main()
