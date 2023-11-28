import pandas as pd
import matplotlib.pyplot as plt
import os


def put_plot():
    files = os.listdir(f'.')
    csv_files = list(filter(lambda x: (len(x) > 4 and
                                       'distance' not in x and
                                       'result' not in x and
                                       x[-4:] == '.csv'
                                       ), files))
    csv_data_list = []
    for file_item in csv_files:
        tmp = pd.read_csv(f'./{file_item}', header=0)
        csv_data_list.append(tmp.dropna())
    dataset = pd.concat(csv_data_list, ignore_index=True)

    px = dataset['alt_x']
    py = dataset['alt_y']
    error = dataset['transformed_error']

    (plt.scatter(px, py, c=error, cmap='plasma', marker='o', edgecolors='black')
     .set_clim(vmin=error.min(), vmax=error.max()))
    plt.colorbar(label='error')
    plt.title('transformation error[m]')
    plt.xlabel('x')
    plt.ylabel('y')

    plt.xlim(200, 1600)
    plt.ylim(800, 200)

    # plt.show()
    plt.savefig('./distance_error_graph.png')


if __name__ == "__main__":
    put_plot()
