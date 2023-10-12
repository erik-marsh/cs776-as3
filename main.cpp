#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#define cimg_display 0
#include "CImg/CImg.h"
#include "Rooms.hpp"
#include "encoding.hpp"

constexpr int NUM_TRIALS = 30;
constexpr int NUM_GENERATIONS = 50;
constexpr int GENERATION_SIZE = 100;
constexpr double CROSSOVER_PROB = 0.7;
constexpr double MUTATION_PROB = 0.001;

struct EvaluationResult
{
    float objective;
    float fitness;
};

struct Individual
{
    Chromosome chromosome;
    float objective;
    float fitness;
};

struct Statistics
{
    std::random_device::result_type seed;
    std::array<Individual, NUM_GENERATIONS + 1> fittestIndividuals;

    // The +1 is so we include the initial generation
    std::array<float, NUM_GENERATIONS + 1> minFitnesses;
    std::array<float, NUM_GENERATIONS + 1> maxFitnesses;
    std::array<float, NUM_GENERATIONS + 1> avgFitnesses;

    std::array<float, NUM_GENERATIONS + 1> minObjective;
    std::array<float, NUM_GENERATIONS + 1> maxObjective;
    std::array<float, NUM_GENERATIONS + 1> avgObjective;
};

using Population = std::array<Individual, GENERATION_SIZE>;
using ProbDist = std::array<double, GENERATION_SIZE>;

Statistics RunGeneticAlgorithm();
void GenerationStatistics(Statistics& stats, Population& population, int gen);
void OutputStatistics(Statistics& stats, std::ostream& csvSummary, std::ostream& bestText,
                      const std::string& bestImageFilename);
float GenerateFloatInRange(std::mt19937& generator, const Range<float> range);
void InitializeIndividual(std::mt19937& generator, Individual& x);
void DrawRoomSet(RoomSet& roomSet, const std::string& filename);
EvaluationResult EvaluateIndividual(RoomSet& rooms);
ProbDist MakeCumulativeProbDist(Population& pop);
std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop);
std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents);
uint8_t MutateBit(std::mt19937& generator, uint8_t bit);

// TODO: do the reliability, quality, speed metrics thing

int main()
{
    // Make sure the data directory exists
    if (!std::filesystem::exists("data")) std::filesystem::create_directory("data");

    // We need to summarize the summary statistics for each generation
    // (The average is over NUM_TRIALS runs)
    std::array<Statistics, NUM_TRIALS> uberStats;

    for (int i = 0; i < NUM_TRIALS; i++)
    {
        Statistics stats = RunGeneticAlgorithm();
        std::stringstream ss;
        ss << "data/stats-trial-" << i << ".csv";
        std::ofstream outFile(ss.str());

        ss.str("");
        ss << "data/best-trial-" << i << ".txt";
        std::ofstream bestText(ss.str());

        ss.str("");
        ss << "data/best-final-gen-trial-" << i << ".png";
        std::string bestImage = ss.str();

        OutputStatistics(stats, outFile, bestText, bestImage);
        uberStats[i] = stats;
    }

    Statistics uberSummary;
    for (int i = 0; i < NUM_GENERATIONS + 1; i++)
    {
        float sumMinObjective = 0.0f;
        float sumMaxObjective = 0.0f;
        float sumAvgObjective = 0.0f;

        float sumMinFitness = 0.0f;
        float sumMaxFitness = 0.0f;
        float sumAvgFitness = 0.0f;

        for (Statistics& stats : uberStats)
        {
            sumMinObjective += stats.minObjective[i];
            sumMaxObjective += stats.maxObjective[i];
            sumAvgObjective += stats.avgObjective[i];

            sumMinFitness += stats.minFitnesses[i];
            sumMaxFitness += stats.maxFitnesses[i];
            sumAvgFitness += stats.avgFitnesses[i];
        }

        uberSummary.minObjective[i] = sumMinObjective / uberStats.size();
        uberSummary.maxObjective[i] = sumMaxObjective / uberStats.size();
        uberSummary.avgObjective[i] = sumAvgObjective / uberStats.size();

        uberSummary.minFitnesses[i] = sumMinFitness / uberStats.size();
        uberSummary.maxFitnesses[i] = sumMaxFitness / uberStats.size();
        uberSummary.avgFitnesses[i] = sumAvgFitness / uberStats.size();
    }

    std::ofstream outFile("data/stats-average.csv");
    std::ofstream bestText("/dev/null");
    OutputStatistics(uberSummary, outFile, bestText, "");

    return 0;
}

Statistics RunGeneticAlgorithm()
{
    Statistics stats;

    std::random_device device{};
    auto seed = device();
    std::mt19937 generator{seed};
    stats.seed = seed;

    std::cout << "Running GA with seed " << static_cast<unsigned int>(seed) << "...\n";

    Population population;
    for (int i = 0; i < population.size(); i++)
    {
        Individual& individual = population[i];
        InitializeIndividual(generator, individual);
        auto roomSet = DecodeChromosome(individual.chromosome);
        auto result = EvaluateIndividual(roomSet);
        individual.objective = result.objective;
        individual.fitness = result.fitness;
    }

    GenerationStatistics(stats, population, 0);

    for (int gen = 0; gen < NUM_GENERATIONS; gen++)
    {
        auto cdf = MakeCumulativeProbDist(population);
        Population newGeneration;

        for (int i = 0; i < GENERATION_SIZE; i += 2)
        {
            auto parents = Select(generator, cdf, population);

            // mutation occurs within the Crossover function
            auto children = Crossover(generator, parents);
            for (auto& c : children)
            {
                auto roomSet = DecodeChromosome(c.chromosome);
                auto result = EvaluateIndividual(roomSet);
                c.objective = result.objective;
                c.fitness = result.fitness;
            }

            newGeneration[i] = children[0];
            newGeneration[i + 1] = children[1];
        }

        GenerationStatistics(stats, newGeneration, gen + 1);
        population = newGeneration;
    }

    return stats;
}

void GenerationStatistics(Statistics& stats, Population& population, int gen)
{
    double minFitness = std::numeric_limits<double>::max();
    double maxFitness = std::numeric_limits<double>::lowest();
    double sumFitness = 0.0;

    double minObjective = std::numeric_limits<double>::max();
    double maxObjective = std::numeric_limits<double>::lowest();
    double sumObjective = 0.0;

    int fittestIndex = 0;

    for (int i = 0; i < population.size(); i++)
    {
        if (population[i].objective < minObjective) minObjective = population[i].objective;
        if (population[i].objective > maxObjective) maxObjective = population[i].objective;
        sumObjective += population[i].objective;

        if (population[i].fitness < minFitness) minFitness = population[i].fitness;
        if (population[i].fitness > maxFitness)
        {
            maxFitness = population[i].fitness;
            fittestIndex = i;
        }
        sumFitness += population[i].fitness;
    }

    stats.minObjective[gen] = minObjective;
    stats.maxObjective[gen] = maxObjective;
    stats.avgObjective[gen] = sumObjective / population.size();

    stats.minFitnesses[gen] = minFitness;
    stats.maxFitnesses[gen] = maxFitness;
    stats.avgFitnesses[gen] = sumFitness / population.size();

    stats.fittestIndividuals[gen] = population[fittestIndex];
}

void OutputStatistics(Statistics& stats, std::ostream& csvSummary, std::ostream& bestText,
                      const std::string& bestImageFilename)
{
    // prints the statistics in a friendly format
    // for consumption by a python script
    csvSummary << std::fixed << std::setprecision(6);
    csvSummary << "MinFitness,MaxFitness,AvgFitness,MinObjective,MaxObjective,AvgObjective\n";
    for (int i = 0; i < stats.avgFitnesses.size(); i++)
    {
        csvSummary << stats.minFitnesses[i] << ",";
        csvSummary << stats.maxFitnesses[i] << ",";
        csvSummary << stats.avgFitnesses[i] << ",";
        csvSummary << stats.minObjective[i] << ",";
        csvSummary << stats.maxObjective[i] << ",";
        csvSummary << stats.avgObjective[i] << "\n";
    }

    // print out the best individuals for each generation to a file
    // and draw out the best individual from the final generation
    //     TODO: this isn't the best overall one
    for (int i = 0; i < stats.fittestIndividuals.size(); i++)
    {
        auto roomSet = DecodeChromosome(stats.fittestIndividuals[NUM_GENERATIONS].chromosome);

        bestText << "========== BEST INDIVIDUAL OF GENERATION " << i << " ==========\n";
        bestText << "Seed.....: " << stats.seed << "\n";
        bestText << "Fitness..: " << stats.fittestIndividuals[NUM_GENERATIONS].fitness << "\n";
        bestText << "Objective: " << stats.fittestIndividuals[NUM_GENERATIONS].objective << "\n";
        PrintChromosome(stats.fittestIndividuals[NUM_GENERATIONS].chromosome, bestText);
        PrintRoomSet(roomSet, bestText);
        bestText << "\n";

        if (i == stats.fittestIndividuals.size() - 1) DrawRoomSet(roomSet, bestImageFilename);
    }
}

float GenerateFloatInRange(std::mt19937& generator, const Range<float> range)
{
    // since we are dealing with discrete float values,
    // we want to generate them as a discrete type first
    constexpr float offset = 10.0f;
    std::uniform_int_distribution<int> dist(range.low * offset, range.high * offset);
    int value = dist(generator);
    return value / offset;
}

void InitializeIndividual(std::mt19937& generator, Individual& x)
{
    constexpr Range<float> defaultRange(0.0f, 102.3f);

    RoomSet roomSet;

    // initialize each room with valid values
    // Living, Kitchen, Bath, Hall, Bed1, Bed2, Bed3
    roomSet[0].type = RoomType::LIVING;
    do
    {
        // fixed proportions must be generated in a different way
        // this prevents us from spinning our wheels in this loop for a Very Long Time
        float tmpLength = GenerateFloatInRange(generator, LIVING_LENGTH);
        float tmpWidth = GenerateFloatInRange(generator, LIVING_WIDTH);
        float propLength = LIVING_PROPORTION * tmpWidth;
        float propWidth = LIVING_PROPORTION * tmpLength;

        // the selection process is biased towards longer width, but whatever
        if (LIVING_LENGTH.Contains(tmpLength) && LIVING_WIDTH.Contains(propWidth))
        {
            roomSet[0].length = tmpLength;
            roomSet[0].width = propWidth;
        }
        else if (LIVING_LENGTH.Contains(propLength) && LIVING_WIDTH.Contains(tmpWidth))
        {
            roomSet[0].length = propLength;
            roomSet[0].width = propWidth;
        }
    } while (!DoesRoomFitConstraints(roomSet[0]));

    roomSet[1].type = RoomType::KITCHEN;
    do
    {
        roomSet[1].length = GenerateFloatInRange(generator, KITCHEN_LENGTH);
        roomSet[1].width = GenerateFloatInRange(generator, KITCHEN_WIDTH);
    } while (!DoesRoomFitConstraints(roomSet[1]));

    roomSet[2].type = RoomType::BATH;
    roomSet[2].length = BATH_LENGTH;
    roomSet[2].width = BATH_WIDTH;

    roomSet[3].type = RoomType::HALL;
    do
    {
        roomSet[3].length = HALL_LENGTH;
        roomSet[3].width = GenerateFloatInRange(generator, HALL_WIDTH);
    } while (!DoesRoomFitConstraints(roomSet[3]));

    roomSet[4].type = RoomType::BED1;
    do
    {
        float tmpLength = GenerateFloatInRange(generator, BED1_LENGTH);
        float tmpWidth = GenerateFloatInRange(generator, BED1_WIDTH);
        float propLength = BED1_PROPORTION * tmpWidth;
        float propWidth = BED1_PROPORTION * tmpLength;

        roomSet[4].length = GenerateFloatInRange(generator, BED1_LENGTH);
        roomSet[4].width = GenerateFloatInRange(generator, BED1_WIDTH);

        if (BED1_LENGTH.Contains(tmpLength) && BED1_WIDTH.Contains(propWidth))
        {
            roomSet[4].length = tmpLength;
            roomSet[4].width = propWidth;
        }
        else if (BED1_LENGTH.Contains(propLength) && BED1_WIDTH.Contains(tmpWidth))
        {
            roomSet[4].length = propLength;
            roomSet[4].width = propWidth;
        }
    } while (!DoesRoomFitConstraints(roomSet[4]));

    roomSet[5].type = RoomType::BED2;
    do
    {
        float tmpLength = GenerateFloatInRange(generator, BED2_LENGTH);
        float tmpWidth = GenerateFloatInRange(generator, BED2_WIDTH);
        float propLength = BED2_PROPORTION * tmpWidth;
        float propWidth = BED2_PROPORTION * tmpLength;

        roomSet[5].length = GenerateFloatInRange(generator, BED2_LENGTH);
        roomSet[5].width = GenerateFloatInRange(generator, BED2_WIDTH);

        if (BED2_LENGTH.Contains(tmpLength) && BED2_WIDTH.Contains(propWidth))
        {
            roomSet[5].length = tmpLength;
            roomSet[5].width = propWidth;
        }
        else if (BED2_LENGTH.Contains(propLength) && BED2_WIDTH.Contains(tmpWidth))
        {
            roomSet[5].length = propLength;
            roomSet[5].width = propWidth;
        }
    } while (!DoesRoomFitConstraints(roomSet[5]));

    roomSet[6].type = RoomType::BED3;
    do
    {
        float tmpLength = GenerateFloatInRange(generator, BED3_LENGTH);
        float tmpWidth = GenerateFloatInRange(generator, BED3_WIDTH);
        float propLength = BED3_PROPORTION * tmpWidth;
        float propWidth = BED3_PROPORTION * tmpLength;

        roomSet[6].length = GenerateFloatInRange(generator, BED3_LENGTH);
        roomSet[6].width = GenerateFloatInRange(generator, BED3_WIDTH);

        if (BED3_LENGTH.Contains(tmpLength) && BED3_WIDTH.Contains(propWidth))
        {
            roomSet[6].length = tmpLength;
            roomSet[6].width = propWidth;
        }
        else if (BED3_LENGTH.Contains(propLength) && BED3_WIDTH.Contains(tmpWidth))
        {
            roomSet[6].length = propLength;
            roomSet[6].width = propWidth;
        }
    } while (!DoesRoomFitConstraints(roomSet[6]));

    // we actually don't care about position at all
    // so let's just generate one randomly and call it a day
    // (because I don't want to re-write my encoders/decoders)
    for (Room& room : roomSet)
    {
        room.x = GenerateFloatInRange(generator, defaultRange);
        room.y = GenerateFloatInRange(generator, defaultRange);
    }

    x.chromosome = EncodeChromosome(roomSet);
}

void DrawRoomSet(RoomSet& roomSet, const std::string& filename)
{
    if (filename == "") return;

    constexpr int backgroundColor = 47;  // shades of gray, i.e. R=G=B=backgroundColor
    static constexpr std::array<uint8_t, 3> borderColor{255, 255, 255};
    static constexpr std::array<uint8_t, 3> invalidColor{191, 191, 191};

    // default matplotlib color cycle
    // ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2']
    static constexpr std::array<uint8_t, 3> livingColor{0x1f, 0x77, 0xb4};
    static constexpr std::array<uint8_t, 3> kitchenColor{0xff, 0x7f, 0x0e};
    static constexpr std::array<uint8_t, 3> bathColor{0x2c, 0xa0, 0x2c};
    static constexpr std::array<uint8_t, 3> hallColor{0xd6, 0x27, 0x28};
    static constexpr std::array<uint8_t, 3> bed1Color{0x94, 0x67, 0xbd};
    static constexpr std::array<uint8_t, 3> bed2Color{0x8c, 0x56, 0x4b};
    static constexpr std::array<uint8_t, 3> bed3Color{0xe3, 0x77, 0xc2};
    static constexpr std::array roomColors = {livingColor, kitchenColor, bathColor, hallColor,
                                              bed1Color,   bed2Color,    bed3Color};

    // maximum possible room length and width is both 20.0f units
    // which is 200px at a precision of 0.1f
    constexpr int squareLength = 200;
    constexpr int paddingPixels = 10;
    constexpr int checkerSize = 5;

    // image will be a 4x2 grid of squares used to visualize room areas
    constexpr int imageLength = (5 * paddingPixels) + (4 * squareLength);
    constexpr int imageWidth = (3 * paddingPixels) + (2 * squareLength);

    std::array<int, NUM_ROOMS> roomOffsetX;
    std::array<int, NUM_ROOMS> roomOffsetY;
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        const int col = i % 4;
        const int row = i / 4;
        roomOffsetX[i] = paddingPixels + (col * (squareLength + paddingPixels));
        roomOffsetY[i] = paddingPixels + (row * (squareLength + paddingPixels));
    }

    // image layout is...
    // x 0 -> 1023 (length)
    // y 0
    // |
    // V 1023 (width)
    cimg_library::CImg<uint8_t> image(imageLength, imageWidth, 1, 3, backgroundColor);
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        Room& room = roomSet[i];

        // draw the frames that the rooms will inhabit
        for (int x = roomOffsetX[i]; x < roomOffsetX[i] + squareLength; x++)
        {
            image(x, roomOffsetY[i], 0, 0) = borderColor[0];
            image(x, roomOffsetY[i], 0, 1) = borderColor[1];
            image(x, roomOffsetY[i], 0, 2) = borderColor[2];

            image(x, roomOffsetY[i] + squareLength, 0, 0) = borderColor[0];
            image(x, roomOffsetY[i] + squareLength, 0, 1) = borderColor[1];
            image(x, roomOffsetY[i] + squareLength, 0, 2) = borderColor[2];
        }

        for (int y = roomOffsetY[i]; y < roomOffsetY[i] + squareLength; y++)
        {
            image(roomOffsetX[i], y, 0, 0) = borderColor[0];
            image(roomOffsetX[i], y, 0, 1) = borderColor[1];
            image(roomOffsetX[i], y, 0, 2) = borderColor[2];

            image(roomOffsetX[i] + squareLength, y, 0, 0) = borderColor[0];
            image(roomOffsetX[i] + squareLength, y, 0, 1) = borderColor[1];
            image(roomOffsetX[i] + squareLength, y, 0, 2) = borderColor[2];
        }

        const int length = static_cast<int>(room.length * 10.0f);
        const int width = static_cast<int>(room.width * 10.0f);

        int xMax = roomOffsetX[i] + length;
        xMax = xMax < imageLength ? xMax : imageLength;
        int yMax = roomOffsetY[i] + width;
        yMax = yMax < imageWidth ? yMax : imageWidth;

        // fill the frame with the room's dimensions
        for (int x = roomOffsetX[i]; x < xMax; x++)
        {
            for (int y = roomOffsetY[i]; y < yMax; y++)
            {
                if (DoesRoomFitConstraints(room))
                {
                    image(x, y, 0, 0) = roomColors[i][0];
                    image(x, y, 0, 1) = roomColors[i][1];
                    image(x, y, 0, 2) = roomColors[i][2];
                }
                else
                {
                    // if the room is invalid, draw it as a checkered gray box
                    // i.e. we only draw the top right and bottom left quarters
                    // of a (2 * checkerSize) square
                    const int xMod = x % (2 * checkerSize);
                    const int yMod = y % (2 * checkerSize);

                    // remember, CImg uses y=0 at the top of the image
                    const bool upperRight = (xMod >= checkerSize && yMod < checkerSize);
                    const bool bottomLeft = (xMod < checkerSize && yMod >= checkerSize);

                    if (upperRight || bottomLeft)
                    {
                        image(x, y, 0, 0) = invalidColor[0];
                        image(x, y, 0, 1) = invalidColor[1];
                        image(x, y, 0, 2) = invalidColor[2];
                    }
                }
            }
        }
    }
    image.save_png(filename.c_str());
}

EvaluationResult EvaluateIndividual(RoomSet& rooms)
{
    // a further modification is needed here
    // invalid rooms should be assessed on an individual basis
    // e.g a layout with one invalid room should be more fit than one with five invalid rooms
    // an invalid room with have an objective value of ${ROOM}_AREA.high
    // (the maximum area and thus maximum cost)

    float objective = 0.0f;
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        Room& room = rooms[i];
        objective += DoesRoomFitConstraints(room) ? RoomCost(room) : INVALID_OBJECTIVE[i];
    }
    float fitness = ObjectiveToFitness(objective);

    EvaluationResult ret;
    ret.objective = objective;
    ret.fitness = fitness;
    return ret;
}

ProbDist MakeCumulativeProbDist(Population& pop)
{
    double totalFitness = 0.0;
    for (Individual& indiv : pop)
        totalFitness += indiv.fitness;
    ProbDist cumulativeProbs;

    // if the total fitness is 0, the entire probability distribution will be 0s
    // to prevent this, we instead return a uniform CDF.
    if (totalFitness == 0.0f)
    {
        float accumulator = 0.1f;
        for (int i = 0; i < cumulativeProbs.size(); i++)
        {
            cumulativeProbs[i] = accumulator;
            accumulator += 0.1f;
        }
        return cumulativeProbs;
    }

    // ProbDist fitnessProportions;  // for debugging
    // ProbDist expectedValues;      // for debugging
    double accumulator = 0.0;

    for (int i = 0; i < GENERATION_SIZE; i++)
    {
        const double fitnessProportion = pop[i].fitness / totalFitness;
        accumulator += fitnessProportion;
        cumulativeProbs[i] = accumulator;

        // for debugging
        // double expectedValue = fitnessProportion * GENERATION_SIZE;
        // expectedValues[i] = expectedValue;
        // fitnessProportions[i] = fitnessProportion;
    }

    return cumulativeProbs;
}

std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop)
{
    std::array<Individual, 2> parents;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < 2; i++)
    {
        double prob = dist(generator);
        for (int j = 0; j < GENERATION_SIZE; j++)
        {
            if (prob <= cdf[j])
            {
                parents[i] = pop[j];
                break;
            }
        }
    }

    return parents;
}

std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents)
{
    std::array<Individual, 2> children;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double prob = dist(generator);
    if (prob <= CROSSOVER_PROB)
    {
        std::uniform_int_distribution<int> distInt(0, CHROMOSOME_BITWIDTH - 1);
        int crossoverIndex = distInt(generator);

        for (int i = 0; i < crossoverIndex; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
        }
        for (int i = crossoverIndex; i < CHROMOSOME_BITWIDTH; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
        }
    }
    else
    {
        for (int i = 0; i < CHROMOSOME_BITWIDTH; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
        }
    }

    return children;
}

uint8_t MutateBit(std::mt19937& generator, uint8_t bit)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double prob = dist(generator);
    if (prob <= MUTATION_PROB)
        return !bit;
    return bit;
}
