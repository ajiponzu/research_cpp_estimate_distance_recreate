import os
import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt

print("input dir_code")
dir_code = input()

files = os.listdir(f'./{dir_code}')
csv_files = list(filter(lambda x:(x[0:6] == 'result' and x[6] != '_' and x[-4:] == '.csv'), files))
csv_data_list = []
for fileitem in csv_files:
    tmp = pd.read_csv(f'{dir_code}/{fileitem}', header=0)
    csv_data_list.append(tmp)
dataset = pd.concat(csv_data_list, ignore_index=False)
# data = pd.read_csv('result_premethod1_graph.csv')
x = dataset['altitude_x'].values
y = dataset['altitude_y'].values
z = dataset['error_distance [m]'].values
print(f"mean: {z.mean()}")
print(f"std: {z.std()}")

plt.rcParams["font.size"] = 22
cmap = plt.cm.get_cmap('Reds')

fig = plt.figure()
axis = fig.add_subplot(1, 1, 1)
axis.invert_yaxis()
axis.set_xlabel('altitude_x')
axis.set_ylabel('altitude_y')
axis.set_title('error_distance [m]')
mappable = axis.scatter(x, y, c=z, cmap=cmap)

fig.colorbar(mappable, ax=axis)
#fig.savefig('')
plt.show()