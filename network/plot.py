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
                    entry = 0
                    count += 1
                    if mode == 0:
                        latency.append(int(line.split(" ")[3]))
                    else:
                        throughput.append(float(line.split(" ")[1]))
                if count == 4:
                    label = 1
                    mode += 1
                    count = 0

    return (latency, throughput)

if __name__ == "__main__":
    if len(sys.argv) < 1 and len(sys.argv) > 4:
        print("Program usage: python plot.py <input_file>")
        sys.exit(-1)
    
    threads = [1, 2, 4, 8]
    
    datatcp = load(sys.argv[1])
    dataudp = load(sys.argv[2])
    latencytcp = [datatcp[0][i] * threads[i] for i in range(0, len(threads))]
    latencyudp = [dataudp[0][i] * threads[i] for i in range(0, len(threads))]
    
    print("Latency TCP:")
    print(latencytcp)

    print("Throughput TCP:")
    print(datatcp[1])
    
    print("Latency UDP:")
    print(latencyudp)

    print("Throughput UDP:")
    print(dataudp[1])
    
    plt.rcdefaults()
    fig, ax = plt.subplots()
    
    plt.grid(color='grey', zorder=0, linestyle='--')
    
    plt.plot(threads, datatcp[1][0:4], 'bo--',
             threads, dataudp[1][0:4], 'ro--')

    ax.set_xlabel('Threads')
    ax.set_ylabel('Mbps')
    
    blue_patch = mpatches.Patch(color='blue', label='TCP throughput')
    red_patch = mpatches.Patch(color='red', label='UDP throughput')

    plt.legend(handles=[blue_patch, red_patch], loc=2)
    
    plt.show()
