import datetime
import sys
import numpy as np
from matplotlib import pyplot as plt

if __name__ == '__main__':
    oneapi_accumulator_dump_path = sys.argv[1] if len(sys.argv) > 1 else "logs/accumulator_dump.log"
    naive_accumulator_dump_path = sys.argv[2] if len(sys.argv) > 2 else "logs/naive_accumulator_dump.log"

    with open(oneapi_accumulator_dump_path, 'r') as file:
        oneapi_accumulator_dump = file.readlines()
    oneapi_accumulator_dump = [[int(j) for j in i.split()] for i in oneapi_accumulator_dump]

    with open(naive_accumulator_dump_path, 'r') as file:
        naive_accumulator_dump = file.readlines()
    naive_accumulator_dump = [[int(j) for j in i.split()] for i in naive_accumulator_dump]

    accumulator_dump_difference = [[oneapi_accumulator_dump[i][j] - naive_accumulator_dump[i][j] for j in range(len(oneapi_accumulator_dump[i]))] for i in range(len(oneapi_accumulator_dump))]

    current_time = datetime.datetime.now()
    name_postfix = f'{current_time.year}_{current_time.month}_{current_time.day}_{current_time.hour}_{current_time.minute}_{current_time.second}'

    accumulator_dump_difference_path = f'logs/accumulator_dump_difference_path_{name_postfix}.log'
    with open(accumulator_dump_difference_path, 'w') as file:
        for i in accumulator_dump_difference:
            for j in i:
                file.write(f'{j} ')
            file.write('\n')

    accumulator_dump_difference_plot_path = f'logs/accumulator_dump_difference_plot_{name_postfix}.png'

    y_length = len(accumulator_dump_difference)
    x_length = len(accumulator_dump_difference[0])
    accumulator_dump_difference_reversed = accumulator_dump_difference.copy()
    accumulator_dump_difference_reversed.reverse()

    plt.clf()
    plt.pcolor(np.linspace(0, x_length - 1, x_length), np.linspace(0, y_length - 1, y_length), accumulator_dump_difference_reversed)
    plt.colorbar()
    plt.xlabel('q over pt')
    plt.ylabel('phi')
    plt.savefig(accumulator_dump_difference_plot_path, dpi=200)


#
# def draw_nodes(nodes, path, x_range=None, y_range=None):
#     x_range = x_range if x_range is not None else (0, nodes.shape[1])
#     y_range = y_range if y_range is not None else (0, nodes.shape[0])
#     x = np.linspace(x_range[0], x_range[1], nodes.shape[1])
#     y = np.linspace(y_range[0], y_range[1], nodes.shape[0])
#
#     plt.clf()
#     plt.pcolor(x, y, nodes)
#     plt.colorbar()
#     plt.xlabel("x")
#     plt.ylabel("y")
#     plt.savefig(path, dpi=200)

