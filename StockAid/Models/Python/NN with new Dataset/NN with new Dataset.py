import os.path
import numpy as np
import pandas as pd
import tensorflow as tf
from StockAlgorithm import StockAlgorithm
import pickle

MODEL_SAVE_FOLDER = "Saves/SavedNN/"


class TrainingCallback(tf.keras.callbacks.Callback):
    def __init__(self, stockPredictor):
        self.stockPredictor = stockPredictor

    def on_epoch_end(self, epoch, logs=None):
        self.stockPredictor.lossValues.append(logs["loss"])
        self.stockPredictor.accuracyValues.append(logs["accuracy"])
        print("EPOCH DONE")


class StockPredictor(StockAlgorithm):
    def __init__(self):
        self.model_name = "Neural Network"
        super().__init__(self.model_name, StockAlgorithm.MLType.SUPERVISED_LEARNING)

        self.loaded = False

        self.model = None

        self.x_data = None
        self.y_data = None

        dataset = pd.read_csv("ProcessedDataSaves/Dataset.csv")
        dataset_features = dataset.filter(["Normalized High", "Moving Average", "Average"])
        dataset_labels = dataset.filter(["Label"])

        # TRAINING DATA

        train_split = int(len(dataset.index) * 0.6)

        train_x = dataset_features[:train_split].values
        train_y = dataset_labels[:train_split].values
        train_x = np.expand_dims(train_x, 1)
        train_y = np.expand_dims(train_y, 1)

        self.training_data = tf.data.Dataset.from_tensor_slices((
            tf.cast(train_x, tf.float32),
            tf.cast(train_y, tf.float32)
        )).batch(128)

        # TESTING DATA

        test_split = train_split + int(len(dataset.index) * 0.2)

        test_x = dataset_features[train_split:test_split].values
        test_y = dataset_labels[train_split:test_split].values
        test_x = np.expand_dims(test_x, 1)
        test_y = np.expand_dims(test_y, 1)

        self.test_data = tf.data.Dataset.from_tensor_slices((
            tf.cast(test_x, tf.float32),
            tf.cast(test_y, tf.float32)
        )).batch(128)

        # VALIDATION DATA

        validation_x = dataset_features[test_split:].values
        validation_y = dataset_labels[test_split:].values
        validation_x = np.expand_dims(validation_x, 1)
        validation_y = np.expand_dims(validation_y, 1)

        self.validation_data = tf.data.Dataset.from_tensor_slices((
            tf.cast(validation_x, tf.float32),
            tf.cast(validation_y, tf.float32)
        )).batch(128)

        self.model = tf.keras.models.Sequential([
            tf.keras.layers.Dense(3, activation='relu'),
            tf.keras.layers.Dense(128, activation='relu'),
            tf.keras.layers.Dense(128, activation='relu'),
            tf.keras.layers.Dense(64, activation='relu'),
            tf.keras.layers.Dense(32, activation='relu'),
            tf.keras.layers.Dense(1)
        ])

        self.model.compile(
            optimizer=tf.keras.optimizers.Adam(),
            loss=tf.keras.losses.MeanSquaredError(),
            metrics=tf.keras.metrics.Accuracy()
        )

    def SaveModel(self, saveName: str, overwrite=False) -> bool:
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
        self.model.save_weights(MODEL_SAVE_FOLDER + saveName, overwrite=overwrite)
        return True

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
            self.model.load_weights(MODEL_SAVE_FOLDER + loadName)
            if self.model:
                print("Succeeded Loading model")
                return True
        print("Failed Loading model")
        return False

    def Initialise(self) -> bool:
        dataset = pd.read_csv("ProcessedDataSaves/Dataset.csv")
        dataset_features = dataset.filter(["Normalized High", "Moving Average", "Average"])
        dataset_labels = dataset.filter(["Label"])

        self.x_data = dataset.filter(["Normalized High", "Moving Average", "Average"])
        self.y_data = dataset.filter(["Label"])

        return True

    def Train(self, epochs):
        self.model.fit(self.training_data, epochs=epochs, validation_data=self.validation_data, callbacks=[TrainingCallback(self)])

    def GetHyperParameters(self) -> map:
        return self.hyperParameters

    def EvaluateModel(self, inputValues: list) -> float:
        print("Not Implemented")
        return 1.0

    def MakePrediction(self, inputValues: list) -> list:
        if not self.model:
            print("Trying to make predictions, but no model is loaded or trained!")
            return []
        print(1)
        stock_data_processed = pd.read_csv("ProcessedDataSaves/IndividualStocks/" + inputValues[0][:-4] + "-processed.csv")

        average_data_processed = pd.read_csv("ProcessedDataSaves/IndividualStocks/Average-processed.csv")


        file_data = pd.read_csv("data/" + inputValues[0])

        startIndex = file_data.index[file_data['Date'] == inputValues[1]][0]
        endIndex = file_data.index[file_data['Date'] == inputValues[2]][0]

        print(2)
        data = file_data.iloc[startIndex:endIndex]

        predictions = np.ndarray(shape=(len(data.index), 3))

        print(len(data.index))
        for i, v in enumerate(data.iloc):
            date = v["Date"]

            processed_row = stock_data_processed.loc[stock_data_processed["Date"] == date]

            norm = processed_row["Normalized High"]
            moving_average = processed_row["Moving Average"]
            total_average = average_data_processed.loc[average_data_processed["Date"] == date]["Average"]

            predictions[i] = (norm, moving_average, total_average)

        predictions = np.expand_dims(predictions, 1)

        processedCut = stock_data_processed[startIndex:endIndex]
        processedCut = processedCut.reset_index()

        preds = self.model.predict(predictions)
        finalPred = []

        for index, pred in enumerate(preds):
            print(pred[0], processedCut["Normalized High"][index])
            if pred[0] > processedCut["Normalized High"][index]:
                finalPred.append(0)
            if pred[0] < processedCut["Normalized High"][index]:
                finalPred.append(2)
            else:
                finalPred.append(1)

        return finalPred

