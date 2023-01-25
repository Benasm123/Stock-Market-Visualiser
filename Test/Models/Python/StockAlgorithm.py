from abc import ABC, abstractmethod, abstractproperty
from enum import IntEnum


class StockAlgorithm(ABC):
    # Abstract parent class that all stock algorithms should subclass.
    class MLType(IntEnum):
        # The machine learning method used.
        REINFORCEMENT_LEARNING = 1
        SUPERVISED_LEARNING = 2
        UNSUPERVISED_LEARNING = 3

    class PredictionType(IntEnum):
        # The machine learning method used.
        BUY = 1
        HOLD = 2
        SELL = 3

    def __init__(self, name: str, algorithmType: MLType):
        self.name = name
        self.algorithmType = algorithmType
        self.description = self.ReadDescriptionFile()
        self.hyperParameters = {}
        self.numberOfStocksUsedToTrain = 0
        self.trainingDateRange = "Not Provided"
        self.rewardValues = []
        self.lossValues = []
        self.accuracyValues = []

    # TODO:Implement
    def ReadDescriptionFile(self) -> str:
        # Read the Description.txt file in the algorithm model and return the contents.
        pass

    # TODO:Implement
    @staticmethod
    def SaveModel(saveName: str) -> bool:
        # Save the current model and weights using the save-name to this algorithms folder path.
        print("Saving as:", saveName)
        return True

    # TODO:Implement
    @staticmethod
    def LoadModel(loadName: str) -> bool:
        # Load the model at the load name in the current algorithms path and set current model values and weights to
        # loaded values.
        print("Loading as:", loadName)
        return True

    @abstractmethod
    def Initialise(self) -> bool:
        # Set up the algorithm for use with predicting
        pass

    @abstractmethod
    def Train(self, epochs) -> float:
        # Train the model with the class hyperparameters
        pass

    @abstractmethod
    def GetHyperParameters(self) -> map:
        # return the hyperparameters of the model in the format -> name: value
        pass

    @abstractmethod
    def EvaluateModel(self, inputValues: list) -> float:
        # Evaluate the current model on all given values.
        pass

    @abstractmethod
    def MakePrediction(self, inputValues: list) -> PredictionType:
        # Return algorithm prediction on the given input values.
        pass

    def GetRewardValues(self):
        return self.rewardValues

    def GetLossValues(self):
        return self.lossValues

    def GetAccuracyValues(self):
        return self.accuracyValues
