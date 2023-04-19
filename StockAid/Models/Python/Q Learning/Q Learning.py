import os

import numpy as np
import pandas as pd
import sys

sys.path.append('../')
from StockAlgorithm import StockAlgorithm

DATA_VALUES_SAVE_NAME = "formated_data_values.pickle"
DATA_LABELS_SAVE_NAME = "formated_data_labels.pickle"
MODEL_SAVE_FOLDER = "q_learning"


class StockPredictor(StockAlgorithm):
    def SaveModel(self, saveName: str) -> bool:
        pass

    def LoadModel(self, loadName: str) -> bool:
        pass

    def __init__(self):
        print("Constructed Class")
        super().__init__("Neural Network With Dropout", StockAlgorithm.MLType.REINFORCEMENT_LEARNING)
        self.model_name = "Neural Network With Dropout"

        self.loaded = False

        self.number_of_inputs = 150
        self.stride = 3
        self.averages_to_take = 10

        self.x_data = None
        self.y_data = None

        self.state_space = None
        # Sell, Buy, Hold
        self.actions = None

        self.learning_rate = None
        self.discount = None
        self.exploration_rate = None

        self.Q = None
        self.buy_price = None

    def policy_(self, mode, state, e_rate=0):
        if mode == "train":
            if np.random.uniform() > e_rate:
                return np.argmax(self.Q[int(state * 10), :])
            else:
                return np.random.choice(self.actions)
        elif mode == "test":
            return np.argmax(self.Q[int(state * 10), :])

    def step_(self, action, stock_price):
        reward = 0
        if action == StockPredictor.PredictionType.SELL:  # Sell
            if self.buy_price:
                reward = stock_price - self.buy_price[0]
                self.buy_price = self.buy_price[1:]
            else:
                reward = 0
        elif action == StockPredictor.PredictionType.BUY:  # Buy
            if not self.buy_price:
                self.buy_price.append(stock_price)
            reward = 0
        else:
            if self.buy_price:
                reward = stock_price - self.buy_price[0]
        return reward

    def train_(self, epochs):
        exploration_decay = 1 / epochs
        for run in range(epochs):
            print("Epoch:", run, "/", epochs)
            for index, stock in enumerate(self.x_data.iloc):
                if not index % 150 == 0:
                    continue
                if index % 1000 == 0:
                    print(np.round((index / (len(self.x_data))) * 100, 3), "%")
                    # print(np.round((index / 100000) * 100, 3), "%")
                state = np.round(stock[0], 1)
                for i in range(1, len(stock)):
                    action = self.policy_("train", state, self.exploration_rate)
                    reward = self.step_(action, stock[i])
                    new_state = np.round(stock[i], 1)
                    self.Q[int(state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), action] = self.Q[int(state * 10)  * (1 + (action == StockPredictor.PredictionType.BUY)), action] + self.learning_rate * (
                            reward + self.discount * np.max(self.Q[int(new_state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), :]) - self.Q[
                        int(state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), action])
                    if action == StockPredictor.PredictionType.SELL:
                        self.Q[int(state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), StockPredictor.PredictionType.BUY] = self.Q[int(state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), StockPredictor.PredictionType.BUY] + self.learning_rate * (
                                reward + self.discount * np.max(self.Q[int(new_state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), :]) - self.Q[
                            int(state * 10), action])
                    state = new_state
                if self.buy_price:
                    reward = stock[i] - self.buy_price[0]
                    self.Q[int(np.round(self.buy_price[0] * (1 + (action == StockPredictor.PredictionType.BUY)), 1) * 10), 1] = self.Q[int(np.round(self.buy_price[0] * (1 + (action == StockPredictor.PredictionType.BUY)),
                                                                                              1) * 10), 1] + self.learning_rate * (
                                                                                  reward + self.discount * np.max(
                                                                              self.Q[int(new_state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), :]) -
                                                                                  self.Q[int(state * 10) * (1 + (action == StockPredictor.PredictionType.BUY)), 1])
                    self.buy_price = []
            if self.exploration_rate > 0.001:
                self.exploration_rate -= exploration_decay

    def predict(self, vals):
        return self.policy_("test", np.round(vals, 1) * 2)

    def Initialise(self) -> bool:
        self.x_data = pd.read_pickle(DATA_VALUES_SAVE_NAME)
        self.y_data = pd.read_pickle(DATA_LABELS_SAVE_NAME)

        self.state_space = np.divide(range(0, 21), 10)
        self.actions = [
            StockPredictor.PredictionType.BUY,
            StockPredictor.PredictionType.HOLD,
            StockPredictor.PredictionType.SELL
        ]

        self.learning_rate = 0.83
        self.discount = 0.93
        self.exploration_rate = 1.0

        self.Q = np.zeros((len(self.state_space), len(self.actions)))
        self.buy_price = []
        return True

    def Train(self, epochs) -> float:
        self.train_(epochs)
        print(self.Q)

    def GetHyperParameters(self) -> map:
        return self.hyperParameters

    def EvaluateModel(self, inputValues: list) -> float:
        print("Not Implemented")
        return 1.0

    def MakePrediction(self, inputValues: list) -> int:
        predictions = []

        file_data = pd.read_csv("data/" + inputValues[0])

        startIndex = file_data.index[file_data['Date'] == inputValues[1]][0]
        endIndex = file_data.index[file_data['Date'] == inputValues[2]][0]

        data = file_data.iloc[startIndex:endIndex]

        for i, v in enumerate(data["High"]):
            print(v / np.max(file_data["High"].values[:i + 1]))
            normalised = v / np.max(file_data["High"].values[:i + 1])
            predictions.append(self.predict(normalised))

        return predictions
