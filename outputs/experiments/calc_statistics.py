import pandas as pd
import csv
import os


def output_csv(output_path: str, merged_csv_data: list):
    with open(output_path, "w", encoding="utf-8", newline="") as fp:
        header_name_list = ["道路番号", "平均値", "最大値", "最小値", "標準偏差"]
        writer = csv.DictWriter(fp, fieldnames=header_name_list)
        writer.writeheader()
        writer.writerows(merged_csv_data)


def handle_speed(ref_dict, ex_num_list, lane_num):
    files = os.listdir(f'.')
    csv_files = list(filter(lambda x: (len(x) > 4 and
                                       'distance' not in x and
                                       x[-5:] in ex_num_list), files))
    csv_data_list = []
    for file_item in csv_files:
        tmp = pd.read_csv(f'./{file_item}', header=0)
        csv_data_list.append(tmp.dropna())
    dataset = pd.concat(csv_data_list, ignore_index=True)

    data_dict = dict()
    data_dict['道路番号'] = lane_num
    data_dict['平均値'] = round(dataset['transformed_error'].values.mean(), 2)
    data_dict['最大値'] = round(dataset['transformed_error'].values.max(), 2)
    data_dict['最小値'] = round(dataset['transformed_error'].values.min(), 2)
    data_dict['標準偏差'] = round(dataset['transformed_error'].values.std(), 2)
    ref_dict['変換誤差'].append(data_dict.copy())

    data_dict = dict()
    data_dict['道路番号'] = lane_num
    data_dict['平均値'] = round(dataset['speed_error'].values.mean(), 2)
    data_dict['最大値'] = round(dataset['speed_error'].values.max(), 2)
    data_dict['最小値'] = round(dataset['speed_error'].values.min(), 2)
    data_dict['標準偏差'] = round(dataset['speed_error'].values.std(), 2)
    ref_dict['走行速度誤差'].append(data_dict.copy())


def handle_distance(ref_dict, ex_num_list, lane_num):
    files = os.listdir(f'.')
    csv_files = list(filter(lambda x: (len(x) > 4 and
                                       'distance' in x and
                                       x[-5:] in ex_num_list), files))
    csv_data_list = []
    for file_item in csv_files:
        tmp = pd.read_csv(f'./{file_item}', header=0)
        csv_data_list.append(tmp.dropna())
    dataset = pd.concat(csv_data_list, ignore_index=True)

    data_dict = dict()
    data_dict['道路番号'] = lane_num
    data_dict['平均値'] = round(dataset['distance_error'].values.mean(), 2)
    data_dict['最大値'] = round(dataset['distance_error'].values.max(), 2)
    data_dict['最小値'] = round(dataset['distance_error'].values.min(), 2)
    data_dict['標準偏差'] = round(dataset['distance_error'].values.std(), 2)
    ref_dict['車間距離誤差'].append(data_dict.copy())


def main(csv_dict: dict, ex_num_list, lane_num):
    for idx, elem in enumerate(ex_num_list):
        ex_num_list[idx] = f"{elem}.csv"

    handle_speed(csv_dict, ex_num_list, lane_num)
    handle_distance(csv_dict, ex_num_list, lane_num)


if __name__ == "__main__":
    file_path_key = {"変換誤差": "transformed", "走行速度誤差": "speed", "車間距離誤差": "distance"}
    csv_dictionary = dict()
    csv_dictionary['変換誤差'] = list()
    csv_dictionary['走行速度誤差'] = list()
    csv_dictionary['車間距離誤差'] = list()

    main(csv_dictionary, [0, 2], 0)
    main(csv_dictionary, [1, 3], 1)
    main(csv_dictionary, [4], 2)
    main(csv_dictionary, [5], 3)

    for key in file_path_key.keys():
        output_csv(f"./experiment_{file_path_key[key]}_result.csv", csv_dictionary[key])
