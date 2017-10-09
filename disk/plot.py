import sys
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

def load(filename):
    latency = []
    throughput = []
    mode = 0
    label = 1
    entry = 0
    count = 0
    with open(filename, "r") as fil:
        for line in fil:
            if label == 1:
                label = 0
            else:
                if entry == 0:
                    entry = 1
                elif entry == 1:
                    if mode == 0:
                        entry = 2
                    else:
                        entry = 0
                        count += 1
                    throughput.append(float(line.split(" ")[1]))
                elif entry == 2:
                    entry = 0
                    count += 1
                    latency.append(int(line.split(" ")[2]))
                if count == 4:
                    label = 1
                    mode += 1
                    count = 0

    return (latency, throughput)

if __name__ == "__main__":
    if len(sys.argv) < 1 and len(sys.argv) > 3:
        print("Program usage: python plot.py <input_file>")
        sys.exit(-1)
    
    threads = [1, 2, 4, 8]
    
    data = load(sys.argv[1])
    #latency = [data[0][i] * threads[i] for i in range(0, len(threads))]
    latency = data[0]

    print("Latency:")
    print(latency)

    print("Throughput:")
    print(data[1])
             
    plt.rcdefaults()
    fig, ax = plt.subplots()
    
    plt.grid(color='grey', zorder=0, linestyle='--')
    
    plt.plot(threads, data[1][0:4], 'bo--',
             threads, data[1][4:8], 'go--',
             threads, data[1][8:12], 'ro--',
             threads, data[1][12:16], 'mo--')
    
    ax.set_xlabel('Threads')
    ax.set_ylabel('MB/s')
    
    blue_patch = mpatches.Patch(color='blue', label='8B block size')
    green_patch = mpatches.Patch(color='green', label='8KB block size')
    red_patch = mpatches.Patch(color='red', label='8MB block size')
    purple_patch = mpatches.Patch(color='purple', label='80MB block size')

    plt.legend(handles=[blue_patch, green_patch, red_patch,
                        purple_patch], loc=6)
    
    plt.show()
