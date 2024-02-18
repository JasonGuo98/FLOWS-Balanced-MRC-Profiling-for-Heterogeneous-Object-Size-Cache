import torch
import torch.nn as nn
import torch.optim as optim
from torch.optim.lr_scheduler import StepLR
import numpy as np
import os
from trace_info import trace_info_list
from torch.utils.data import DataLoader
import subprocess
import argparse
import matplotlib.pyplot as plt
import csv
import pandas as pd
import matplotlib.pyplot
import random

def setup_seed(seed):
     torch.manual_seed(seed)
    #  torch.cuda.manual_seed_all(seed)
     np.random.seed(seed)
     random.seed(seed)
    #  torch.backends.cudnn.deterministic = True

class ThreeLayerPerceptron(nn.Module):
    def __init__(self, input_dim, hidden_dim, output_dim):
        super(ThreeLayerPerceptron, self).__init__()
        self.fc1 = nn.Linear(input_dim, hidden_dim, )
        self.relu = nn.ReLU()
        self.fc2 = nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = nn.Linear(hidden_dim, output_dim)

    def forward(self, x):
        x = self.fc1(x)
        x = self.relu(x)
        x = self.fc2(x)
        x = self.relu(x)
        x = self.fc3(x)
        return x

def load_one_data(idx):
    trace_info_dict = trace_info_list[idx]
    sample_method = "FIX_RATE"
    sample_metric = "1"
    seed = 0
    trace_path = trace_info_dict["path"]
    total_access_num = trace_info_dict["total_access_num"]
    total_access_size = trace_info_dict["total_access_size"]
    unique_access_num = trace_info_dict["unique_access_num"]
    unique_access_size = trace_info_dict["unique_access_size"]
    min_item_size = trace_info_dict["min_item_size"]
    file_name = os.path.basename(trace_path)

    y_file = f"profile_res-{file_name}-REAL-{sample_method}-{sample_metric}-bin-MAEQ-{seed}.csv"
    y_file_path = os.path.join("./results", y_file)


    x_file = f"profile_res-{file_name}-bin-MAEQ.txt"
    x_file_path = os.path.join("./features", x_file)

    try:
        data = pd.read_csv(y_file_path)
    except:
        return None, None
    y = data.iloc[:, 1:]
    y = np.array(y)
    y0 = y[:,0]
    y1 = y[:,1]
    y0 = np.array(y0)
    y1 = np.array(y1)

    y = np.concatenate((y0[::], y1[::]))
        
    if(not os.path.exists(x_file_path)):
        return None, None
    x = np.loadtxt(x_file_path)

    x0 = x[:2]
    x1 = x[2:2002]
    
    x2 = x[2002:]

    x =np.concatenate((x0, x1, x2))
    return x, y

def load_data():
    XX, YY = [], []
    for i in range(len(trace_info_list)):
        x, y = load_one_data(i)
        if(x is not None):
            XX.append(x)
            YY.append(y)
    
    XX = np.array(XX)
    YY = np.array(YY)

    X = torch.from_numpy(XX).float()
    y = torch.from_numpy(YY).float()

    return X, y

    

def save_model(model, file_path):
    torch.save(model.state_dict(), file_path)

def load_model(model, file_path):
    model.load_state_dict(torch.load(file_path))

def one_exp(exp_id):
    setup_seed(exp_id)
    
    X, y = load_data()

    input_dim = X.shape[-1]
    hidden_dim = 1024
    output_dim = y.shape[-1]

    model = ThreeLayerPerceptron(input_dim, hidden_dim, output_dim)

    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.1)

    scheduler = StepLR(optimizer, step_size=40, gamma=0.1)

    batch_size = min(4, X.shape[0])

    dataset = torch.utils.data.TensorDataset(X, y)  # 假设X和y是训练数据和标签
    data_loader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    for epoch in range(200):
        for batch_data, batch_labels in data_loader:
            batch_data[:,2:2002] += np.random.normal(0, 0.001, (len(batch_data), 2000))
            batch_labels += np.random.normal(0, 0.001, (len(batch_data), 2000)).astype(float)
            batch_labels = batch_labels.float()


            optimizer.zero_grad()
            output = model(batch_data)
            loss = criterion(output, batch_labels)
            loss.backward()
            optimizer.step()

        scheduler.step()

        print(f"Epoch: {epoch+1}, Loss: {loss.item()}, Learning Rate: {scheduler.get_lr()[0]}")


    output = model(X).detach().numpy()

    method = "DL"
    sample_method = "NONE"
    sample_metric = "1"
    tracetype = "bin"
    metric = "MAEQ"
    seed = exp_id
    i = 0
    for trace_info_dict in trace_info_list:
        trace_path = trace_info_dict["path"]
        total_access_num = trace_info_dict["total_access_num"]
        total_access_size = trace_info_dict["total_access_size"]
        unique_access_num = trace_info_dict["unique_access_num"]
        unique_access_size = trace_info_dict["unique_access_size"]
        min_item_size = trace_info_dict["min_item_size"]

        file_name = os.path.basename(trace_path)

        y_file = f"profile_res-{file_name}-REAL-FIX_RATE-1-bin-MAEQ-0.csv"
        y_file_path = os.path.join("./results", y_file)


        real_mrc = pd.read_csv(y_file_path)
        predict_pd = real_mrc.iloc[::1]

        predict_mrc = output[i]

        predict_bmrc = predict_mrc[:len(predict_pd)]
        predict_omrc = predict_mrc[len(predict_pd):]


        predict_pd[" BMRC_ratio"] = predict_bmrc
        predict_pd[" OMRC_ratio"] = predict_omrc

        output_file = f"profile_res-{file_name}-{method}-{sample_method}-{sample_metric}-{tracetype}-{metric}-{seed}.csv"
        output_file_path = os.path.join("results", output_file)
        predict_pd.to_csv(output_file_path, index = None)



    # save model
    save_model(model, f"trained_model/model{exp_id}.pt")

for seed in range(21):
    one_exp(seed)