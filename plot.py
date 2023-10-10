from sys import argv
from os import path
import csv
import matplotlib.pyplot as plt

if __name__ == "__main__":
    if len(argv) != 3:
        print("Usage: python3 plot.py <data directory> <output directory>")
        quit()
    
    dataDir = argv[1]
    outDir = argv[2]

    for i in range(30):
        inputFile = f"fitnessStatsRun{i}.csv"
        outputFitnessFile = f"fitnessRun{i}.png"
        outputObjectiveFile = f"objectiveRun{i}.png"
        inputRelPath = path.join("./", dataDir, inputFile)
        outputRelPathFitness = path.join("./", outDir, outputFitnessFile)
        outputRelPathObjective = path.join("./", outDir, outputObjectiveFile)

        with open(inputRelPath) as f:
            reader = csv.reader(f)
            next(reader)

            minFitness = []
            maxFitness = []
            avgFitness = []
            minObjective = []
            maxObjective = []
            avgObjective = []

            for row in reader:
                vals = [float(x) for x in row]
                minFitness.append(vals[0])
                maxFitness.append(vals[1])
                avgFitness.append(vals[2])
                minObjective.append(vals[3])
                maxObjective.append(vals[4])
                avgObjective.append(vals[5])

            fitnessFigure = plt.figure()
            fitnessAxes = fitnessFigure.subplots()
            fitnessAxes.plot(minFitness, label="Min fitness", color="orangered")
            fitnessAxes.plot(maxFitness, label="Max fitness", color="green")
            fitnessAxes.plot(avgFitness, label="Average fitness", color="steelblue")
            fitnessAxes.legend()
            fitnessAxes.set_ylim([0, 100])
            fitnessAxes.set_xlabel("Generation")
            fitnessAxes.set_ylabel("Fitness (higher is better)")
            fitnessFigure.savefig(outputRelPathFitness)
            plt.close()

            objectiveFigure = plt.figure()
            objectiveAxes = objectiveFigure.subplots()
            objectiveAxes.plot(minObjective, label="Min objective", color="green")
            objectiveAxes.plot(maxObjective, label="Max objective", color="orangered")
            objectiveAxes.plot(avgObjective, label="Average objective", color="steelblue")
            objectiveAxes.legend()
            objectiveAxes.set_xlabel("Generation")
            objectiveAxes.set_ylabel("Objective function value (lower is better)")
            objectiveFigure.savefig(outputRelPathObjective)
            plt.close()
