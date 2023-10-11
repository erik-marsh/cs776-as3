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
void OutputStatistics(Statistics& stats, std::ostream& outStream);
float GenerateFloatInRange(std::mt19937& generator, const Range<float> range);
void InitializeIndividual(std::mt19937& generator, Individual& x);
void DrawRoomSet(RoomSet& roomSet);
// float GetFitness(RoomSet& rooms);
EvaluationResult EvaluateIndividual(RoomSet& rooms);
ProbDist MakeCumulativeProbDist(Population& pop);
std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop);
std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents);
uint8_t MutateBit(std::mt19937& generator, uint8_t bit);

// TODO: do the reliability, quality, speed metrics thing
// TODO: keep a log of the best individuals for each generation

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
        std::stringstream filename;
        filename << "data/stats-trial-" << i << ".csv";
        std::ofstream outFile(filename.str());
        OutputStatistics(stats, outFile);
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
    OutputStatistics(uberSummary, outFile);

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
        // std::cout << "\n\n==================== INDIVIDUAL " << i << "====================\n";
        // PrintChromosome(individual.chromosome);
        auto roomSet = DecodeChromosome(individual.chromosome);
        // PrintRoomSet(roomset);

        // individual.objective = ObjectiveFunction(roomSet);
        // individual.fitness = GetFitness(roomSet);
        auto result = EvaluateIndividual(roomSet);
        individual.objective = result.objective;
        individual.fitness = result.fitness;
    }

    GenerationStatistics(stats, population, 0);
    // std::cout << "Finished initialization...\n";

    for (int gen = 0; gen < NUM_GENERATIONS; gen++)
    {
        auto cdf = MakeCumulativeProbDist(population);
        Population newGeneration;

        for (int i = 0; i < GENERATION_SIZE; i += 2)
        {
            // std::cout << "\nSelecting...\n";
            auto parents = Select(generator, cdf, population);
            // for (auto& p : parents)
            // {
            //     std::cout << "     Selected ";
            //     for (auto val : p.chromosome)
            //         std::cout << (int)val;
            //     std::cout << " (Fitness=" << p.fitness << ")\n";
            // }

            // mutation occurs within the Crossover function
            // std::cout << "Reproducing...\n";
            auto children = Crossover(generator, parents);
            for (auto& c : children)
            {
                auto roomSet = DecodeChromosome(c.chromosome);
                // c.objective = ObjectiveFunction(roomSet);
                // c.fitness = GetFitness(roomSet);
                auto result = EvaluateIndividual(roomSet);
                c.objective = result.objective;
                c.fitness = result.fitness;

                // std::cout << "    Got child ";
                // for (auto val : c.chromosome)
                //     std::cout << (int)val;
                // std::cout << " (Fitness=" << c.fitness << ")\n";
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

    for (auto& indiv : population)
    {
        if (indiv.objective < minObjective) minObjective = indiv.objective;
        if (indiv.objective > maxObjective) maxObjective = indiv.objective;
        sumObjective += indiv.objective;

        if (indiv.fitness < minFitness) minFitness = indiv.fitness;
        if (indiv.fitness > maxFitness) maxFitness = indiv.fitness;
        sumFitness += indiv.fitness;
    }

    stats.minObjective[gen] = minObjective;
    stats.maxObjective[gen] = maxObjective;
    stats.avgObjective[gen] = sumObjective / population.size();

    stats.minFitnesses[gen] = minFitness;
    stats.maxFitnesses[gen] = maxFitness;
    stats.avgFitnesses[gen] = sumFitness / population.size();
}

void OutputStatistics(Statistics& stats, std::ostream& outStream)
{
    // prints the statistics in a friendly format
    // for consumption by a python script
    outStream << std::fixed << std::setprecision(6);
    outStream << "MinFitness,MaxFitness,AvgFitness,MinObjective,MaxObjective,AvgObjective\n";
    for (int i = 0; i < stats.avgFitnesses.size(); i++)
    {
        outStream << stats.minFitnesses[i] << ",";
        outStream << stats.maxFitnesses[i] << ",";
        outStream << stats.avgFitnesses[i] << ",";
        outStream << stats.minObjective[i] << ",";
        outStream << stats.maxObjective[i] << ",";
        outStream << stats.avgObjective[i] << "\n";
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

    // i don't currently care about position,
    // so we can initialize them all randomly in one go
    // for (int i = 0; i < NUM_ROOMS; i++)
    // {
    //     bool didCollide;
    //     // keep attempting to generate rooms until none of them collide
    //     do
    //     {
    //         didCollide = false;
    //         Range<float> xRange(0.0f, 102.3f - roomSet[i].length);
    //         Range<float> yRange(0.0f, 102.3f - roomSet[i].width);
    //         roomSet[i].x = GenerateFloatInRange(generator, xRange);
    //         roomSet[i].y = GenerateFloatInRange(generator, yRange);

    //         // should not execute for i = 0
    //         for (int j = 0; j < i; j++)
    //         {
    //             if (DoRoomsCollide(roomSet[j], roomSet[i]))
    //             {
    //                 didCollide = true;
    //                 break;
    //             }
    //         }
    //     } while (didCollide);
    // }

    // we actually don't care about position at all
    // so let's just generate one randomly and call it a day
    // (because I don't want to re-write my encoders/decoders)
    for (Room& room : roomSet)
    {
        room.x = GenerateFloatInRange(generator, defaultRange);
        room.y = GenerateFloatInRange(generator, defaultRange);
    }

    x.chromosome = EncodeChromosome(roomSet);
    DrawRoomSet(roomSet);
    PrintRoomSet(roomSet);
    PrintChromosome(x.chromosome);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void DrawRoomSet(RoomSet& roomSet)
{
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

        const int xMax = roomOffsetX[i] + length;
        const int yMax = roomOffsetY[i] + width;

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
                    image(x, y, 0, 0) = invalidColor[0];
                    image(x, y, 0, 1) = invalidColor[1];
                    image(x, y, 0, 2) = invalidColor[2];
                }
            }
        }
    }
    image.save_png("out.png");
}

// float GetFitness(RoomSet& rooms)
// {
//     // if at least one constraint is not met,
//     // the entire chromosome is invalid and should have close to zero fitness
//     // (we can't use pure zero because of the way the CDF is calculated)
//     constexpr float invalidFitness = 0.0f;

//     bool doesFit = true;
//     for (Room& room : rooms)
//     {
//         if (!DoesRoomFitConstraints(room))
//         {
//             // std::cout << "Objective: Invalid\n";
//             // std::cout << "Fitness..: " << invalidFitness << "\n";
//             return invalidFitness;
//         }
//     }

//     float objective = doesFit ? ObjectiveFunction(rooms) : 0.0f;
//     // std::cout << "Objective: " << objective << "\n";
//     float fitness = ObjectiveToFitness(objective);
//     // std::cout << "Fitness..: " << fitness << "\n";
//     return fitness;
// }

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
    // std::cout << "Total fitness: " << totalFitness << "\n";
    ProbDist cumulativeProbs;  // need

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

    ProbDist fitnessProportions;  // for bookkeeping only
    ProbDist expectedValues;      // for bookkeeping only
    double accumulator = 0.0;

    for (int i = 0; i < GENERATION_SIZE; i++)
    {
        double fitnessProportion = pop[i].fitness / totalFitness;
        fitnessProportions[i] = fitnessProportion;

        accumulator += fitnessProportion;
        cumulativeProbs[i] = accumulator;

        double expectedValue = fitnessProportion * GENERATION_SIZE;
        expectedValues[i] = expectedValue;
    }

    // for (int i = 0; i < GENERATION_SIZE; i++)
    // {
    //     for (auto val : pop[i].GetFirstDouble())
    //         std::cout << (int)val;
    //     std::cout << " | ";
    //     for (auto val : pop[i].GetSecondDouble())
    //         std::cout << (int)val;
    //     std::cout << " | Fitness: " << pop[i].fitness << " => Proportion: ";
    //     std::cout << fitnessProportions[i] << " => Expected parents: ";
    //     std::cout << expectedValues[i] << " => Rounded: ";
    //     int exptectedInt = static_cast<int>(std::round(expectedValues[i]));
    //     std::cout << exptectedInt << " |\tCumulative prob: ";
    //     std::cout << cumulativeProbs[i] << "\n";
    // }

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
        // std::cout << "Crossover occured! (Index=" << crossoverIndex << ")\n";

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
    {
        // std::cout << "Mutation occured!\n";
        return !bit;
    }
    return bit;
}
