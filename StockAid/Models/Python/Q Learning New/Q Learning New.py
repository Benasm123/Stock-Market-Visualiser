import ctypes
import os.path
import time
from ctypes import Structure
from enum import IntEnum
from multiprocessing import Process, freeze_support, sharedctypes, Pool

import multiprocessing
import _multiprocessing
import runpy

from QTable import QTable

import numpy as np
import pandas as pd
from StockAlgorithm import StockAlgorithm
import pickle

MODEL_SAVE_FOLDER = "Saves/SavedQLearning/"

NORM_HIGH = 0
MOVING_AVG = 1
HIGH = 2
ACT_NEXT_DAY = 3
AVERAGE = 4


def QTableAtAction(q_table, state, action):
    state_index = np.array(np.multiply(state, 10), dtype=int)
    return q_table[state_index[0]][state_index[1]][state_index[2]][action]


def QTableAtState(q_table, state):
    state_index = np.array(np.multiply(state, 10), dtype=int)
    return q_table[state_index[0]][state_index[1]][state_index[2]]


def SetQTableAt(q_table, state, action, value):
    state_index = np.array(np.multiply(state, 10), dtype=int)
    q_table[state_index[0]][state_index[1]][state_index[2]][action] = value


def TrackProgress(progress_array, total):
    while True:
        print("\rProgress : {:.2%}".format((np.average(progress_array) / total)), end="")
        time.sleep(1)


def QLearn(og_q_table, q_table_array, reward_array, evaluation_array, progress_array, index, learning_rate, discount,
           exploration_rate, training_stocks, validation_stocks):
    reward_cum = 0
    q_table = og_q_table
    for stock_index, stock in enumerate(training_stocks):
        progress_array[index] = stock_index + 1
        buy_prices = []
        state = (
            np.round(stock[0][NORM_HIGH], 1),
            np.round(stock[0][MOVING_AVG], 1),
            np.round(stock[0][AVERAGE], 1)
        )

        for value_index, values in enumerate(stock[1:]):
            action_taken = Policy([StockPredictor.Action.BUY, StockPredictor.Action.HOLD, StockPredictor.Action.SELL],
                                  q_table, state, exploration_rate)

            reward_for_action = Reward(action_taken, values[HIGH:ACT_NEXT_DAY + 1], buy_prices)

            if action_taken == StockPredictor.Action.BUY:
                buy_prices.append(values[HIGH])
            elif action_taken == StockPredictor.Action.SELL:
                buy_prices.clear()

            reward_cum += reward_for_action

            next_state = (
                np.round(values[NORM_HIGH], 1),
                np.round(values[MOVING_AVG], 1),
                np.round(values[AVERAGE], 1)
            )

            q_value = QTableAtAction(q_table.value, state, action_taken) + learning_rate * (
                    reward_for_action + discount * (
                np.max(QTableAtState(q_table.value, next_state))
            ) - QTableAtAction(q_table.value, state, action_taken)
            )

            SetQTableAt(q_table.value, state, action_taken, q_value)

            state = next_state
        # break
        # Clear buy prices every unique stock.
        # TODO -> Sell remaining bought stocks, or find a way to give reward!
        buy_prices.clear()

    evaluation_array[index] = EvaluateQTable(validation_stocks, q_table)
    q_table_array[index] = q_table
    reward_array[index] = sharedctypes.ctypes.c_float(reward_cum)


def Policy(actions, q_table, state, exploration_rate=0.0):
    """ Return the action to perform. If exploration is not given will always pick best. """
    if np.random.uniform() > exploration_rate:
        return np.argmax(QTableAtState(q_table.value, state))
    else:
        return np.random.choice(actions)


def Reward(action_taken, real_prices, buy_prices):
    """ Return the reward for performing the given action. """
    # On a buy, add the price to buy prices to keep track for when we sell.
    # Give negative reward to discourage excessive trading.
    # Using -5% of the buy price.
    if action_taken == StockPredictor.Action.BUY:
        return real_prices[0] * -0.01
    # If we hold, and we have no stocks bought, return the difference between today and tomorrow's price squared.
    if action_taken == StockPredictor.Action.HOLD:
        return (real_prices[0] - real_prices[1]) / 2
    # If we sell return the squared sum of all prices, and clear the buy prices list.
    if action_taken == StockPredictor.Action.SELL:
        sum_profit_of_sell = 0
        for buy_price in buy_prices:
            sum_profit_of_sell += real_prices[0] - buy_price
        return sum_profit_of_sell


def EvaluateQTable(data, q_table):
    reward_cum = 0
    for stock_index, stock in enumerate(data):
        buy_prices = []

        for index, values in enumerate(stock):
            state = (
                np.round(values[NORM_HIGH], 1),
                np.round(values[MOVING_AVG], 1),
                np.round(values[AVERAGE], 1)
            )
            action_taken = Policy([StockPredictor.Action.BUY, StockPredictor.Action.HOLD, StockPredictor.Action.SELL],
                                  q_table, state)
            reward_cum += Reward(action_taken, values[HIGH:], buy_prices)
    return reward_cum


class StockPredictor(StockAlgorithm):
    """ Q-Learning algorithm for stock action predictions."""

    class Action(IntEnum):
        BUY = 0
        HOLD = 1
        SELL = 2

    def __init__(self):
        # Parent init
        self.model_name = "Q Learning"
        super().__init__(self.model_name, StockAlgorithm.MLType.REINFORCEMENT_LEARNING)

        # Action available to the RL algorithm.
        self.actions = [self.Action.BUY, self.Action.HOLD, self.Action.SELL]

        # The table of states that the algorithm can be in.
        # Using --> Normal High, 60 Day Average, 500 Stock Average <-- all normalised between 0.0 and 1.0.
        # Rounding to the closest value times 10 to store the state.
        self.state_space = np.zeros([11, 11, 11])

        # Q Table that keeps track of Q-value of all state-action pairs
        self.q_table = None

        # Hyperparameters
        self.learning_rate = 0.83
        self.discount = 0.93
        self.exploration_rate = 1.0

        # Data Loading
        if not os.path.exists("../../../ProcessedDataSaves/Dataset.csv"):
            return

        dataset = pd.read_csv("../../../ProcessedDataSaves/Dataset.csv")
        dataset_features = dataset.filter(["Normalized High", "Moving Average", "Average"])

        average_stocks = pd.read_csv("../../../ProcessedDataSaves/IndividualStocks/Average-processed.csv")
        stock_files = os.listdir("../../../ProcessedDataSaves/IndividualStocks")
        stock_files.remove("Average-processed.csv")

        self.stocks = []
        for stock_file in stock_files:
            stock_df = pd.read_csv(("../../../ProcessedDataSaves/IndividualStocks/" + stock_file)).filter(
                ["Date", "Normalized High", "Moving Average", "High", "Actual Next Day"])
            stock_df = stock_df.merge(average_stocks, on="Date", how="left")
            stock_df = stock_df.drop(["Date"], axis=1)
            self.stocks.append(stock_df.values)

        train_split = int(len(self.stocks) * 0.6)
        test_split = train_split + (int(len(self.stocks) * 0.2))

        self.training_stocks = self.stocks[:train_split]
        self.testing_stocks = self.stocks[train_split:test_split]
        self.validating_stocks = self.stocks[test_split:]

        # Training Values
        # Buy prices to keep track of how much to reward on sell.
        self.buy_prices = []

    def Initialise(self) -> bool:
        pass

    # TODO -> Look into making the epoch only evaluate reward after each stock, and add
    # TODO -> reward after an episode to all actions taken during that episode. Might
    # TODO -> need more epochs to get good results, but might promote better habits.
    def Train(self, epochs):
        if not self.q_table:
            self.q_table = QTable()
        # Implement some better decay function
        exploration_decay = 1.0 / epochs
        for epoch in range(1, epochs + 1):
            print("Epoch: {}/{}".format(epoch, epochs))
            # Start processes which run Q Learning on current Q-Table values.
            # After all complete evaluate performance on validation set, and select highest performing one by reward.
            # Select this versions Q-Table as the new Q-Table to send to all new processes, and repeat the process.
            number_of_processes = 12
            processes = []
            q_table_array = sharedctypes.Array(QTable, number_of_processes)
            evaluation_array = sharedctypes.Array(sharedctypes.ctypes.c_float, number_of_processes)
            reward_array = sharedctypes.Array(sharedctypes.ctypes.c_float, number_of_processes)
            progress_array = sharedctypes.Array(sharedctypes.ctypes.c_int, number_of_processes)

            for i in range(number_of_processes):
                process = Process(target=QLearn, args=(
                    self.q_table,
                    q_table_array,
                    reward_array,
                    evaluation_array,
                    progress_array,
                    i,
                    self.learning_rate,
                    self.discount,
                    self.exploration_rate,
                    self.training_stocks,
                    self.validating_stocks
                ))
                process.start()
                processes.append(process)

            progress_process = Process(target=TrackProgress, args=(progress_array, len(self.training_stocks)))
            progress_process.start()

            for process in processes:
                process.join()

            progress_process.terminate()

            evaluation = np.array(evaluation_array)
            print(evaluation)
            best_index = np.argmax(evaluation)
            self.q_table = q_table_array[best_index]
            self.rewardValues.append(np.float32(reward_array[best_index]))

            self.SaveModel("Checkpoint")

            # Adjust exploration rate after every epoch to reduce how much we explore.
            self.exploration_rate -= exploration_decay
            # self.rewardValues.append(reward_cum)
        print("")

    def GetHyperParameters(self) -> map:
        pass

    def EvaluateModel(self, inputValues: list) -> float:
        pass

    def MakePrediction(self, inputValues: list) -> StockAlgorithm.PredictionType:
        stock_data_processed = pd.read_csv(
            "ProcessedDataSaves/IndividualStocks/" + inputValues[0][:-4] + "-processed.csv")

        average_data_processed = pd.read_csv("ProcessedDataSaves/IndividualStocks/Average-processed.csv")

        file_data = pd.read_csv("data/" + inputValues[0])

        startIndex = file_data.index[file_data['Date'] == inputValues[1]][0]
        endIndex = file_data.index[file_data['Date'] == inputValues[2]][0]

        data = file_data.iloc[startIndex:endIndex]

        predictions = np.ndarray(shape=(len(data.index), 3))

        for i, v in enumerate(data.iloc):
            date = v["Date"]

            processed_row = stock_data_processed.loc[stock_data_processed["Date"] == date]

            norm = processed_row["Normalized High"]
            moving_average = processed_row["Moving Average"]
            total_average = average_data_processed.loc[average_data_processed["Date"] == date]["Average"]

            predictions[i] = (norm, moving_average, total_average)

        finalPred = []

        for pred in predictions:
            finalPred.append(Policy(self.actions, self.q_table, pred))

        return finalPred

    def SaveModel(self, saveName: str) -> bool:
        if not os.path.exists("Saves"):
            os.mkdir("Saves")
        if not os.path.exists(MODEL_SAVE_FOLDER):
            os.mkdir(MODEL_SAVE_FOLDER)
        if not os.path.exists(MODEL_SAVE_FOLDER + saveName + "-metrics"):
            os.mkdir(MODEL_SAVE_FOLDER + saveName + "-metrics")
        if self.accuracyValues:
            with open(MODEL_SAVE_FOLDER + saveName + "-metrics/accuracy-save", "wb") as accuracy_file:
                pickle.dump(self.accuracyValues, accuracy_file)
        if self.lossValues:
            with open(MODEL_SAVE_FOLDER + saveName + "-metrics/loss-save", "wb") as loss_file:
                pickle.dump(self.lossValues, loss_file)
        if self.rewardValues:
            with open(MODEL_SAVE_FOLDER + saveName + "-metrics/reward-save", "wb") as reward_file:
                pickle.dump(self.rewardValues, reward_file)
        with open(MODEL_SAVE_FOLDER + saveName, "wb") as saveFile:
            pickle.dump(self.q_table, saveFile)

    def LoadModel(self, loadName: str) -> bool:
        if os.path.exists(MODEL_SAVE_FOLDER + loadName + "-metrics/accuracy-save"):
            with open(MODEL_SAVE_FOLDER + loadName + "-metrics/accuracy-save", "rb") as accuracy_file:
                self.accuracyValues = pickle.load(accuracy_file)

        if os.path.exists(MODEL_SAVE_FOLDER + loadName + "-metrics/loss-save"):
            with open(MODEL_SAVE_FOLDER + loadName + "-metrics/loss-save", "rb") as loss_file:
                self.lossValues = pickle.load(loss_file)

        if os.path.exists(MODEL_SAVE_FOLDER + loadName + "-metrics/reward-save"):
            with open(MODEL_SAVE_FOLDER + loadName + "-metrics/reward-save", "rb") as reward_file:
                self.rewardValues = pickle.load(reward_file)

        if os.path.exists(MODEL_SAVE_FOLDER + loadName):
            with open(MODEL_SAVE_FOLDER + loadName, "rb") as model_file:
                self.q_table = pickle.load(model_file)
                # print(pickle.load(model_file))

        print(np.array(self.q_table.value))
        print(self.rewardValues)


if __name__ == '__main__':
    freeze_support()
    sp = StockPredictor()
    sp.Train(500)
    sp.SaveModel("500E")
    sp.LoadModel("500E")
