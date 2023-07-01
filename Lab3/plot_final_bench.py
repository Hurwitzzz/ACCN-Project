import matplotlib.pyplot as plt
import numpy as np

K = "K"
Name = "Name"
TotalTime = "TotalTime"

# time ./lab3.bin 2 0
naive = [
0.035000,
7179.870117,
6489.686035,
4635.078125,
9852.003906
]


#time ./lab3.bin 2 1
fft = [
0.696000,
0,
0,
0,
0,
]

#time ./lab3.bin 2 2
win = [
0.447000,
2327.107056,
2185.979980,
1956.439026,
4145.481934
]

# time ./lab3.bin 2 3
auto = [
0.060000,
2424.878906,
3084.159058,
1951.655029,
4000.984009
]


fig, axs = plt.subplots(2, 2)



nets = [
"testnet",
"smallnet",
"mediumnet",
"largenet",
"giantnet"
]

ax = axs[(0, 0)]
ax.bar(nets, naive, )

ax.set_ylabel('Total Time (ms)')
ax.set_title('Naive')

ax = axs[(0, 1)]
ax.bar(nets, win, )

ax.set_ylabel('Total Time (ms)')
ax.set_title('Winograd')

ax = axs[(1, 0)]
ax.bar(nets, fft, )

ax.set_ylabel('Total Time (ms)')
ax.set_title('FFT')

ax = axs[(1, 1)]
ax.bar(nets, auto, )

ax.set_ylabel('Total Time (ms)')
ax.set_title('Automatic')

plt.show()

'''
#for a in naive:
axs[(0, 0)].plot(a[TotalTime], label=a[Name])
#for a in fft:
axs[(0, 1)].plot(a[TotalTime], label=a[Name])
#for a in win:
axs[(1, 0)].plot(a[TotalTime], label=a[Name])
#for a in auto:
axs[(1, 1)].plot(a[TotalTime], label=a[Name])



for a in [(0, 0), (0, 1), (1, 0), (1, 1)]:
    ax = axs[a]
    ax.legend(loc='best')
    ax.set(xlabel='Net', ylabel='Total. time (ms)')
    #title=str(ic) + " input / " + str(oc) + " output channels")
    ax.grid()



plt.show()
'''
