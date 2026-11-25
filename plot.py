#!/usr/bin/env python3

import pandas as pd
import seaborn as sns
import sys
import matplotlib.pyplot as plt

if len(sys.argv) != 2:
    print("Usage: python plot.py <input_file>")
    sys.exit(1)

input_file = sys.argv[1]
data = pd.read_csv(input_file)

plt.figure()
ax1 = sns.barplot(x="counter", y="value", hue="type", data=data)

plt.xlabel("Counter")
plt.ylabel("Value")
ax1.set_yscale("log")
plt.title("Comparison of Counters: Baseline vs. Multi Threaded")
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()
# plt.savefig("plot_comparison.pdf")
plt.show()
plt.close()

baseline = data[data["type"] == "baseline"].set_index("counter")["value"]
data["ratio"] = data["value"] / data["counter"].map(baseline)

multi = data[data["type"] == "multi threaded"]

plt.figure()
ax2 = sns.barplot(x="counter", y="ratio", data=multi)

plt.axhline(1, color="red", linestyle="--", label="Baseline")
plt.xlabel("Counter")
plt.ylabel("Ratio (multi threaded / baseline)")
ax2.set_yscale("log")
plt.title("Multi Threaded relative to Baseline")
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()
# plt.savefig("plot_ratio.pdf")
plt.show()
plt.close()