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

ax = sns.barplot(x="counter", y="value", hue="type", data=data)

# Add labels and title
plt.xlabel("Counter")
plt.ylabel("Value")
ax.set_yscale("log")
plt.title("Comparison of Counter Before and After")
plt.legend()

# Better readability
plt.xticks(rotation=45)
plt.tight_layout()

# Save the plot to a file
# plt.savefig("plot_comparison.pdf") # speichern in pdf
plt.show()
plt.close()


# Bonus: ratio

baseline = data[data["type"] == "baseline"].set_index("counter")["value"]

data["ratio"] = data["value"] / data["counter"].map(baseline)

plt.figure()

optimized = data[data["type"] == "optimized"]
sns.barplot(x="counter", y="ratio", data = optimized)

plt.axhline(1, color="red", linestyle="--", label="Baseline")
plt.xlabel("Counter")
plt.ylabel("Ratio")
ax.set_yscale("log")
plt.title("optimize realtive to baseline")
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()
# plt.savefig("plot_ratio.pdf")
plt.show()