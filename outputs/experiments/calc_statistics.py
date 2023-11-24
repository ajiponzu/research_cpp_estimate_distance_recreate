import pandas as pd
import csv
import os


def output_csv(output_path: str, merged_csv_data: list):
    with open(output_path, "w", encoding="utf-8", newline="") as fp:
        header_name_list = ["実験番号", "平均値", "最大値", "最小値", "標準偏差"]
        writer = csv.DictWriter(fp, fieldnames=header_name_list)
        writer.writeheader()
        writer.writerows(merged_csv_data)


def handle_speed(ref_dict, ex_num):
    files = os.listdir(f'.')
    csv_files = list(filter(lambda x: (len(x) > 4 and 'distance' not in x and x[-5:] == f'{ex_num}.csv'), files))
    csv_data_list = []
    for file_item in csv_files:
        tmp = pd.read_csv(f'./{file_item}', header=0)
        csv_data_list.append(tmp.dropna())
    dataset = pd.concat(csv_data_list, ignore_index=True)

    data_dict = dict()
    data_dict['実験番号'] = ex_num
    data_dict['平均値'] = dataset['transformed_error'].values.mean()
    data_dict['最大値'] = dataset['transformed_error'].values.max()
    data_dict['最小値'] = dataset['transformed_error'].values.min()
    data_dict['標準偏差'] = dataset['transformed_error'].values.std()
    ref_dict['変換誤差'].append(data_dict.copy())

    data_dict = dict()
    data_dict['実験番号'] = ex_num
    data_dict['平均値'] = dataset['speed_error'].values.mean()
    data_dict['最大値'] = dataset['speed_error'].values.max()
    data_dict['最小値'] = dataset['speed_error'].values.min()
    data_dict['標準偏差'] = dataset['speed_error'].values.std()
    ref_dict['走行速度誤差'].append(data_dict.copy())


def handle_distance(ref_dict, ex_num):
    dataset = pd.read_csv(f'./experiment_distance{ex_num}.csv').dropna()
    data_dict = dict()
    data_dict['実験番号'] = ex_num
    data_dict['平均値'] = dataset['distance_error'].values.mean()
    data_dict['最大値'] = dataset['distance_error'].values.max()
    data_dict['最小値'] = dataset['distance_error'].values.min()
    data_dict['標準偏差'] = dataset['distance_error'].values.std()
    ref_dict['車間距離誤差'].append(data_dict.copy())


def main(csv_dict: dict, ex_num):
    handle_speed(csv_dict, ex_num)
    handle_distance(csv_dict, ex_num)


if __name__ == "__main__":
    file_path_key = {"変換誤差": "transformed", "走行速度誤差": "speed", "車間距離誤差": "distance"}
    csv_dictionary = dict()
    csv_dictionary['変換誤差'] = list()
    csv_dictionary['走行速度誤差'] = list()
    csv_dictionary['車間距離誤差'] = list()

    main(csv_dictionary, 0)
    main(csv_dictionary, 1)
    main(csv_dictionary, 3)
    main(csv_dictionary, 4)
    main(csv_dictionary, 5)

    for key in file_path_key.keys():
        output_csv(f"./experiment_{file_path_key[key]}_result.csv", csv_dictionary[key])
