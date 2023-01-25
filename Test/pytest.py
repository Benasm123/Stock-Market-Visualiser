
import numpy as np
import os
import pandas as pd
import matplotlib.pyplot as plt
import tensorflow as tf

DATA_VALUES_SAVE_NAME = "formated_data_values.pickle"
DATA_LABELS_SAVE_NAME = "formated_data_labels.pickle"

class stock_predictor:
    def __init__(self):
        self.model_name = "Neural Network"

        self.number_of_inputs = 150
        self.stride = 3
        self.averages_to_take = 10

        if not os.path.exists(DATA_VALUES_SAVE_NAME):
            create_dataset(self.number_of_inputs, self.stride, self.averages_to_take)

        self.x_data = pd.read_pickle(DATA_VALUES_SAVE_NAME)
        self.y_data = pd.read_pickle(DATA_LABELS_SAVE_NAME)

        self.model = self.create_model()

### TEST
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
        self.train_values = train_dataset.shuffle(100).batch(64)

        self.evaluation_values = tf.data.Dataset.from_tensor_slices((test_x, test_y)).batch(64)

    def create_model(self):
        model = tf.keras.models.Sequential([
            tf.keras.layers.Conv1D(16, kernel_size=5, activation="relu"),
            tf.keras.layers.MaxPooling1D(),
            tf.keras.layers.Flatten(),
            tf.keras.layers.Dense(128, activation="relu"),
            tf.keras.layers.Dense(2, activation="softmax")
        ])

        model.compile(
            optimizer=tf.keras.optimizers.Adam(),
            loss=tf.keras.losses.SparseCategoricalCrossentropy(),
            metrics=tf.keras.metrics.SparseCategoricalAccuracy()
        )

        return model

    def train_model(self, epochs=1, validation_dataset=None):
        self.train_values = self.model.fit(self.train_values, epochs=epochs, validation_data=validation_dataset)

    def evaluate_model(self, evaluate_dataset):
        self.evaluation_values = model.evaluate(evaluate_dataset)

    def save_model_weights(self):
        self.model.save_weights("./Models/Weights/" + self.model_name)

    def load_model_weights(self):
        self.model.load_weights("./Models/Weights/" + self.model_name)

    def make_prediction(self, x_vals):
        # return np.random.randint(2)
        return self.model.predict(x_vals, verbose=0)

    def get_number_of_inputs(self):
        return self.number_of_inputs

def create_dataset(vals_per_input, spacing, average_to_take):
    DATA_FOLDER_NAME = "data"

    data_x = []
    data_y = []

    print("PATH IS HERE: ", os.path.abspath("."))
    with os.scandir(DATA_FOLDER_NAME) as stocks:
        for index, stock in enumerate(stocks):
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

def traind():
    DATA_FOLDER_NAME = "data"
    SPACING = 3
    DATE_RANGE = 150
    AVERAGE_DATE_RANGE = 10

    data_x = []
    data_y = []

    print("PATH IS HERE: ", os.path.abspath("."))
    with os.scandir(DATA_FOLDER_NAME) as stocks:
        for index, stock in enumerate(stocks):
            stock_prices = pd.read_csv(DATA_FOLDER_NAME + "/" + stock.name, encoding="utf-8")
            stock_prices = stock_prices.dropna()["High"]
            stock_prices = stock_prices.values
            end_point = stock_prices.size - (DATE_RANGE + AVERAGE_DATE_RANGE)
            start_price = np.max(stock_prices) * 0.4
            if end_point <= 0:
                continue
            current_point = 0
            while current_point < end_point:
                x = stock_prices[current_point : current_point + DATE_RANGE]
                average_after = np.average(stock_prices[current_point + DATE_RANGE : current_point + DATE_RANGE + AVERAGE_DATE_RANGE])
                y = x[-1] <= average_after
                x = np.divide(x, np.amax(x))

                data_x.append(x)
                data_y.append(y)

                current_point += SPACING

    data_x = np.asarray(data_x)
    data_y = np.asarray(data_y)

    print("DONE")

    split_percent = 0.8

    split_index = int(data_y.size * split_percent)

    train_x = data_x[:split_index]
    train_y = data_y[:split_index]
    test_x = data_x[split_index:]
    test_y = data_y[split_index:]

    train_x = np.expand_dims(train_x, 2)[:-10000]
    train_y = np.expand_dims(train_y, 1)[:-10000]

    test_x = np.expand_dims(test_x, 2)
    test_y = np.expand_dims(test_y, 1)

    train_dataset = tf.data.Dataset.from_tensor_slices((train_x, train_y))
    train_dataset = train_dataset.shuffle(100).batch(1)

    test_dataset = tf.data.Dataset.from_tensor_slices((test_x, test_y)).batch(1)

    model = tf.keras.models.Sequential([
        tf.keras.layers.Conv1D(32, kernel_size=5, activation="relu"),
        tf.keras.layers.MaxPooling1D(),
        tf.keras.layers.Conv1D(16, kernel_size=5, activation="relu"),
        tf.keras.layers.MaxPooling1D(),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(128, activation="relu"),
        tf.keras.layers.Dense(528, activation="relu"),
        tf.keras.layers.Dense(528, activation="relu"),
        tf.keras.layers.Dense(528, activation="relu"),
        tf.keras.layers.Dense(152, activation="relu"),
        tf.keras.layers.Dense(2, activation="softmax")
    ])

    model.compile(optimizer=tf.keras.optimizers.SGD(),
                  loss=tf.keras.losses.SparseCategoricalCrossentropy(),
                  metrics=tf.keras.metrics.SparseCategoricalAccuracy())

    nn_results = model.fit(train_dataset, epochs=3, validation_data=(data_x[-10000:], data_y[-10000:]))

    model.evaluate(test_dataset)

number = 1

def predict_today():
    global number
    number += 1
    return 0

def predict_all():
    return number

stock_model = None

def setup():
    global stock_model
    stock_model = stock_predictor()

def train():
    global stock_model
    stock_model.train_model(5)

def predict(*args):
    global stock_model
    return np.argmax(stock_model.make_prediction(np.expand_dims(np.divide([args], np.max([args]), 2))[0]))

setup()
train()