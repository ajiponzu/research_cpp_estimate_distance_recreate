import numpy as np
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
    for file_item in csv_files:
        file_name, file_ext = os.path.splitext(os.path.basename(file_item))
        dataset = pd.read_csv(f'./{file_item}', header=0).dropna()
        data_time = [i * 0.2 for i in range(len(dataset))]

        speed = dataset['speed']
        speed_correct = dataset['speed_correct']
        error = dataset['speed_error']

        plt.clf()
        plt.xlabel('time[s]')
        plt.ylabel('speed[km/h]')
        plt.ylim(0, 110)
        plt.yticks(np.arange(0, 110, 10))

        plt.plot(data_time, speed, label='speed', color='blue')
        plt.plot(data_time, speed_correct, label='speed_correct', color='red')
        plt.bar(data_time, error, color='green', label='error')
        plt.legend(loc='upper right', fontsize=12)

        # plt.show()
        plt.savefig(f'{file_name}_speed.png')


if __name__ == "__main__":
    put_plot()
