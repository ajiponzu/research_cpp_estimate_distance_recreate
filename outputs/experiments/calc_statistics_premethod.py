import pandas as pd
import csv
import os


def output_csv(output_path: str, merged_csv_data: list):
    with open(output_path, "w", encoding="utf-8", newline="") as fp:
        header_name_list = ["実験番号", "変換誤差(m)", "走行速度誤差(km/h)", "車間距離誤差(m)"]
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
    ref_dict['変換誤差(m)'] = dataset['transed_error'].values.mean()
    ref_dict['走行速度誤差(km/h)'] = dataset['speed_error'].values.mean()


def handle_distance(ref_dict, ex_num):
    dataset = pd.read_csv(f'./experiment_distance{ex_num}.csv').dropna()
    ref_dict['車間距離誤差(m)'] = dataset['distance_error'].values.mean()


def main(ex_num):
    csv_dict = dict()
    csv_dict['実験番号'] = ex_num
    handle_speed(csv_dict, ex_num)
    handle_distance(csv_dict, ex_num)
    return csv_dict


if __name__ == "__main__":
    csv_data_list = list()
    # csv_data_list.append(main(0))
    # csv_data_list.append(main(1))
    # csv_data_list.append(main(2))
    csv_data_list.append(main(3))
    # csv_data_list.append(main(4))
    # csv_data_list.append(main(5))
    output_csv("./experiment_result.csv", csv_data_list)
