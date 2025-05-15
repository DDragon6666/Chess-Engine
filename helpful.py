import matplotlib.pyplot as plt

start_time = 600_000 # 10 minutes
increment = 5_000 # 5 seconds


def get_time_used(x):
    if (x < 10000): return min(x, increment + 100)
    return x / 20 + increment

times = []


time = start_time

for i in range(200):
    print(i, time / 1000)
    times.append(time / 1000)
    time -= get_time_used(time)
    time += increment


times.append(time / 1000)

plt.plot([i for i in range(len(times))], times)
plt.show()
