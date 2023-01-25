import numpy as np
import os
import pandas as pd
import matplotlib.pyplot as plt
import tensorflow as tf
import sys

sys.path.append('../')
from StockAlgorithm import StockAlgorithm

DATA_VALUES_SAVE_NAME = "formated_data_values.pickle"
DATA_LABELS_SAVE_NAME = "formated_data_labels.pickle"
MODEL_SAVE_FOLDER = "nn_model_with_dropout"


class StockPredictor(StockAlgorithm):
    def Initialise(self) -> bool:
        print("DEEP INIT")
        return False

    def Train(self, epochs) -> float:
        print("DEEP TRAIN")
        self.train_model(epochs)
        return 1.0

    def GetHyperParameters(self) -> map:
        print("DEEP HP")
        return {}

    def EvaluateModel(self, inputValues: list) -> float:
        print("DEEP EVAL")
        self.evaluate_model()
        print(self.evaluation_results)
        return self.evaluation_results[0]

    def MakePrediction(self, inputValues: list) -> int:
        vals = np.reshape(inputValues, (1, 15, 1))
        vals = np.divide(vals, np.max(inputValues))
        val = (self.make_prediction(vals))
        return np.argmax(val)

    def __init__(self):
        self.model_name = "Neural Network With Dropout"

        self.loaded = False

        self.number_of_inputs = 15
        self.stride = 3
        self.averages_to_take = 1

        if not os.path.exists(DATA_VALUES_SAVE_NAME):
            create_dataset(self.number_of_inputs, self.stride, self.averages_to_take)

        self.x_data = pd.read_pickle(DATA_VALUES_SAVE_NAME)
        self.y_data = pd.read_pickle(DATA_LABELS_SAVE_NAME)

        print(self.x_data[0])

        self.model = self.create_model()

        split_percent = 0.8

        split_index = int(self.y_data.size * split_percent)

        train_x = self.x_data[:split_index]
        train_y = self.y_data[:split_index]
        test_x = self.x_data[split_index:]
        test_y = self.y_data[split_index:]

        train_x = np.expand_dims(train_x, 2)[:-10000]
        train_y = np.expand_dims(train_y, 1)[:-10000]

        test_x = np.expand_dims(test_x, 2)
        test_y = np.expand_dims(test_y, 1)

        train_dataset = tf.data.Dataset.from_tensor_slices((train_x, train_y))
        self.train_values = train_dataset.shuffle(100).batch(4)
        self.train_results = None

        self.evaluation_values = tf.data.Dataset.from_tensor_slices((test_x, test_y)).batch(4)
        self.evaluation_results = None

    def create_model(self):
        model = tf.keras.models.Sequential([
            tf.keras.layers.Conv1D(2, kernel_size=3, activation="relu"),
            tf.keras.layers.Conv1D(50, kernel_size=13, activation="relu"),
            tf.keras.layers.Conv1D(2, kernel_size=1, activation="relu"),
            tf.keras.layers.Flatten(),
            tf.keras.layers.Dense(2, activation="softmax")
        ])

        model.compile(
            optimizer=tf.keras.optimizers.SGD(),
            loss=tf.keras.losses.SparseCategoricalCrossentropy(),
            metrics=tf.keras.metrics.SparseCategoricalAccuracy()
        )

        return model

    def train_model(self, epochs=1, validation_dataset=None):
        self.train_results = self.model.fit(self.train_values, epochs=epochs, validation_data=validation_dataset)

    def evaluate_model(self):
        self.evaluation_results = self.model.evaluate(self.evaluation_values)

    def save_model_weights(self):
        self.model.save_weights("./Models/Python/Deeper_Conv_NN/Weights/" + self.model_name)

    def load_model_weights(self):
        self.model.load_weights("./Models/Python/Deeper_Conv_NN/Weights/" + self.model_name)
        self.loaded = True

    def make_prediction(self, x_vals):
        return self.model.predict(x_vals, verbose=0)

    def get_number_of_inputs(self):
        return self.number_of_inputs


def create_dataset(vals_per_input, spacing, average_to_take):
    DATA_FOLDER_NAME = "data"

    vals_per_input = int(vals_per_input)
    spacing = int(spacing)
    average_to_take = int(average_to_take)

    data_x = []
    data_y = []

    print("PATH IS HERE: ", os.path.abspath("."))
    with os.scandir(DATA_FOLDER_NAME) as stocks:
        for index, stock in enumerate(stocks):
            if stock.name == "AAPL.csv":
                continue
            stock_prices = pd.read_csv(DATA_FOLDER_NAME + "/" + stock.name, encoding="utf-8")
            stock_prices = stock_prices.dropna()["High"]
            stock_prices = stock_prices.values
            end_point = stock_prices.size - (vals_per_input + average_to_take)
            start_price = np.max(stock_prices) * 0.4
            if end_point <= 0:
                continue
            current_point = 0
            while current_point < end_point:
                x = stock_prices[current_point: current_point + vals_per_input]
                average_after = np.average(
                    stock_prices[current_point + vals_per_input: current_point + vals_per_input + average_to_take]
                )
                y = x[-1] <= average_after
                x = np.divide(x, np.amax(x))

                data_x.append(x)
                data_y.append(y)

                current_point += spacing

    data_x = np.asarray(data_x)
    data_y = np.asarray(data_y)

    df_x = pd.DataFrame(data_x)
    df_y = pd.DataFrame(data_y)

    df_x.to_pickle(DATA_VALUES_SAVE_NAME)
    df_y.to_pickle(DATA_LABELS_SAVE_NAME)
