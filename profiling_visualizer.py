import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import os

###########################################
###           Calculation Part          ###
###########################################

#reading file time_points_data.txt

dirname, fname = os.path.split(os.getcwd())
#dirname, fname = os.path.split(dirname)
# path = os.path.join(dirname, r"x64", r"Release", r"time_points_data.txt")
path = os.path.join(dirname, r"ompt_profiler", r"benchmarks", r"jakobi", r"__time_points_data.txt")


print(r"Reading ", path, r" compleated sucsessfully")
points = np.loadtxt(path)

#for the span selector
x = []
y = []
x_min = points[0][1]
x_max = points[-1][1]
span_step = 0.001 #sec
span_step = max(span_step, (x_max - x_min)/100000)
# min_step = 0.001
# iterator = 0
# buf = 0
# for row in points:
#     if iterator != 0:
#         if min_step > abs(row[1] - buf):
#             min_step = abs(row[1] - buf)
#         buf = row[1]
#     else:
#         buf = row[1]
#     iterator += 1
# span_step = min(span_step, min_step)
iterator = 0
for row in points:
    if iterator != 0:
        while (row[1] - x[-1] >= span_step):
            x.append(x[-1]+span_step)
            y.append(y[-1])
    x.append(row[1])
    y.append(row[2])
    iterator += 1
x = np.array(x)
y = np.array(y)

#data mining
begin_threads = []
begin_threads_buf = []

threads = []

end_threads = []
end_threads_buf = []

begin_parallel_regions = []
parallel_regions = []
end_parallel_regions = []

id_threads = []
efficiency_of_threads = []

work_scope_arrays = []
work_scope_arrays_effectiveness = []
work_scope_arrays_effectiveness_buf = []

#extracting time datas and sharing by their types
for row in points:
    if (row[0] == 1):
        begin_threads_buf.append(row)
    if (row[0] == 2):
        end_threads_buf.append(row)
    if (row[0] == 3):
        begin_parallel_regions.append(row[1])
    if (row[0] == 4):
        end_parallel_regions.append(row[1])

buffer = []

flag = 0
number_corrupts = 0
for row in points:
    if (row[0] == 3):
        flag += 1
    if (row[0] == 20):
        buffer.append(row)
        work_scope_arrays_effectiveness.append(row)

    if (row[0] == 4 and flag != 1):
        print('error data of working threads')
        exit()

    if (row[0] == 4 and flag == 1 and len(buffer) == len(end_threads_buf)*2):
        work_scope_arrays.append(buffer)
        buffer = []
        flag = 0

    if (row[0] == 4 and flag == 1 and len(buffer) != len(end_threads_buf)*2):
        #print('corrupt data of working threads')
        flag = 0
        number_corrupts += 1

print("corrupts number:")
print(number_corrupts)


#sorting time datas
n = 2 #id thread index from data
def sort_col(i):
    return i[n]

begin_threads_buf.sort(key=sort_col)
for i in range(len(begin_threads_buf)):
    begin_threads.append(begin_threads_buf[i][1])

end_threads_buf.sort(key=sort_col)
for i in range(len(end_threads_buf)):
    end_threads.append(end_threads_buf[i][1])


for i in range(len(work_scope_arrays)):
    work_scope_arrays[i].sort(key=sort_col)
    #print(work_scope_arrays[i])

for i in range(len(work_scope_arrays_effectiveness)):
    work_scope_arrays_effectiveness.sort(key=sort_col)
    #print(work_scope_arrays_effectiveness[i])

#calculating efficiency
for i in range(len(begin_threads)):
    threads.append(end_threads[i] - begin_threads[i])
for i in range(len(begin_parallel_regions)): #old version
    parallel_regions.append(end_parallel_regions[i] - begin_parallel_regions[i])


number = len(parallel_regions) * 2
for i in range(len(threads)): #new version
    buf = []
    for j in range(number):
        buf.append(work_scope_arrays_effectiveness[i*number+j][1])
    work_scope_arrays_effectiveness_buf.append(buf)
work_scope_arrays_effectiveness.clear()
for j in range(len(work_scope_arrays_effectiveness_buf)):
    buf = []
    for i in range(int(len(work_scope_arrays_effectiveness_buf[j])/2)):
        buf.append(
            work_scope_arrays_effectiveness_buf[j][2*i+1] - work_scope_arrays_effectiveness_buf[j][2*i])
    work_scope_arrays_effectiveness.append(buf)

sum_time_of_work_regions = []
for i in range(len(threads)):
    sum_time_of_work_regions.append(np.sum(work_scope_arrays_effectiveness[i]))


id_threads = np.arange(1, len(threads) + 1, 1)
#sum_time_of_parallel_regions = np.sum(parallel_regions) #old version
for i in range(len(id_threads)):
    # buf = sum_time_of_parallel_regions/threads[i] #old version
    # efficiency_of_threads.append(buf)
    buff = []
    buf = sum_time_of_work_regions[i]/threads[i]  # new version
    buff.append(buf)
    efficiency_of_threads.append(buff)

print(efficiency_of_threads)


###################################################
###              Graphical part                 ###
###################################################

#1.0
# # Map value to color
# color_mapper = np.vectorize(lambda x: {0: 'red', 1: 'blue', 2:'green'}.get(x))
#
# # Plot a line for every line of data in your file
# #Parallel regions
# c = np.ones(len(begin_parallel_regions))
# cc = np.linspace(0, 0, len(begin_parallel_regions))
# for i in range(1, len(id_threads)+1):
#     if (i == 1):        #условие нужно для того, чтобы убрать лишние подписи из легенды
#         plt.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=500,
#                    label="parallel region")
#     else:
#         plt.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=500)
#
# #threads lifetime
# c = np.zeros(len(id_threads))
# plt.hlines(id_threads, begin_threads, end_threads, colors=color_mapper(c), lw=15, label="thread")
#
# #thread work time
# c = 2*np.ones(len(id_threads))
# cc = np.linspace(0, 0, len(begin_parallel_regions))
# for i in range(1, len(id_threads)+1):
#     if (i == 1):        #условие нужно для того, чтобы убрать лишние подписи из легенды
#         plt.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=10,
#                    label="working thread")
#     else:
#         plt.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=10)
#
#
#
# plt.title('Timeline chart')
# plt.ylabel('Номер потока')
# plt.yticks(np.arange(1, len(id_threads)+1, step=1))
# plt.xlabel('Время, с')
# plt.xlim(left=points[0][1], right=points[-1][1])
# plt.tight_layout()
# plt.grid(which='major', axis='y', color='gray', linewidth=2)
# plt.grid(which='both', axis='x', linestyle='--', linewidth=0.5)
# plt.legend(loc='lower left', shadow=True, title="Time of:", markerscale=0.0003)
# mpl.rcParams["legend.markerscale"] = 0.2
# plt.show()

#1.1
#эксперименты

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import SpanSelector, MultiCursor, Slider, Button, RadioButtons


fig, (ax1, ax2) = plt.subplots(2, figsize=(8, 6))

ax1.set(facecolor='#FFFFCC')
multi = MultiCursor(fig.canvas, (ax1, ax2), color='aqua', lw=1.2)
ax1.set_title("Timeline chart")

# Map value to color
color_mapper = np.vectorize(lambda x: {0: 'red', 1: 'blue', 2: 'limegreen'}.get(x))

def draw_everything(ax1):
    # Plot a line for every line of data in your file
    # Parallel regions
    c = np.ones(len(begin_parallel_regions))
    cc = np.linspace(0, 0, len(begin_parallel_regions))
    for i in range(1, len(id_threads) + 1):
        if (i == 1):  # условие нужно для того, чтобы убрать лишние подписи из легенды
            ax1.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=500,
                       label="parallel region")
        else:
            ax1.hlines(cc + i, begin_parallel_regions, end_parallel_regions, colors=color_mapper(c), lw=500)

    # threads lifetime
    c = np.zeros(len(id_threads))
    ax1.hlines(id_threads, begin_threads, end_threads, colors=color_mapper(c), lw=15, label="thread")

    # thread work time
    c = 2 * np.ones(len(id_threads))
    cc = np.linspace(1, 1, int(len(work_scope_arrays[-1])))
    for i in range(len(work_scope_arrays)):
        for j in range(len(work_scope_arrays[i])):
            if j % 2 == 0:
                if (j == 0):  # условие нужно для того, чтобы убрать лишние подписи из легенды
                   ax1.hlines(cc + j/2, work_scope_arrays[i][j], work_scope_arrays[i][j + 1],
                              colors=color_mapper(c),
                              lw=10,
                              label="working thread")
                else:
                    ax1.hlines(cc + j/2, work_scope_arrays[i][j], work_scope_arrays[i][j + 1],
                               colors=color_mapper(c),
                               lw=10)


    ax1.set_ylabel("Номер потока")
    ax1.set_yticks(np.arange(1, len(id_threads) + 1, step=1))
    ax1.set_xlabel("Время, с")
    ax1.set_xlim(left=points[0][1], right=points[-1][1])
    ax1.set_ylim(y.min() - 0.5, y.max() + 0.5)

    ax1.grid(which='major', axis='y', color='gray', linewidth=2)
    ax1.grid(which='both', axis='x', linestyle='--', linewidth=0.5)
    #ax1.legend(loc='lower left', shadow=True, title="Time of:", markerscale=0.0003)
    #mpl.rcParams["legend.markerscale"] = 0.2
    ax1.invert_yaxis()

draw_everything(ax1)
ax1.plot()


ax2.set(facecolor='#FFFFCC')
draw_everything(ax2)
ax2.plot()

#внести правки по выделению меньше расстояния между точками
def onselect(xmin, xmax):
    indmin, indmax = np.searchsorted(x, (xmin, xmax))
    indmax = min(len(x), indmax+1)

    thisx = x[indmin - 1:indmax + 1]
    thisy = y[indmin:indmax]

    ax2.set_xlim(thisx[0], thisx[-1])
    #ax2.set_ylim(thisy.min() - 0.5, thisy.max() + 0.5) #чтобы реализовать выбор потока на осмотр
    ax2.set_ylim(y.min() - 0.5, y.max() + 0.5)
    ax2.invert_yaxis()
    fig.canvas.draw()

def resetonselect():
    indmin, indmax = 0, -1
    indmax = min(len(x), indmax+1)

    thisx = x[indmin:indmax]
    thisy = y[indmin:indmax]

    ax2.set_xlim(x.min(), x.max())
    ax2.set_ylim(y.min() - 0.5, y.max() + 0.5)
    ax2.invert_yaxis()
    fig.canvas.draw()

# Set useblit=True on most backends for enhanced performance.
span = SpanSelector(ax1, onselect, 'horizontal', useblit=True,
                    rectprops=dict(alpha=0.5, facecolor='aqua'))


axcolor = 'lightgoldenrodyellow'
resetax = plt.axes([0.01, 0.025, 0.1, 0.04])
button = Button(resetax, 'Reset', color=axcolor, hovercolor='0.975')
def reset(event):
    resetonselect()
button.on_clicked(reset)

#experiments
columns = ("Efficiency of threads")
# Add a table at the bottom of the axes
the_table = ax1.table(cellText=efficiency_of_threads,
                      #rowLabels=rows,
                      #rowColours=colors,
                      colWidths=[0.2] * 1,
                      colLabels=columns,
                      loc='left')

plt.subplots_adjust(left=0.2, bottom=0.1)
plt.tight_layout()
plt.show()

