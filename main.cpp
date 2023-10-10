#include <array>
#include <cmath>
#include <cstdint>
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

constexpr int NUM_GENERATIONS = 250;  // 50;
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
// float GetFitness(RoomSet& rooms);
EvaluationResult EvaluateIndividual(RoomSet& rooms);
ProbDist MakeCumulativeProbDist(Population& pop);
std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop);
std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents);
uint8_t MutateBit(std::mt19937& generator, uint8_t bit);

int main()
{
    // struct Individual:
    //     std::vector<uint8_t> chromosome;
    //     // decoded chromosome value (not necessary), aka std::vector<double>
    //     double fitness;
    //     indices(?) of parents (not necessary?)

    // struct Population:
    //     // arrays of populations, pointer swap trick
    //     // metrics like chromosome length and generation number
    //     // fitness metrics
    //     // probabilities for stuff
    //     // indices of the best and worst fitnesses
    //     // metrics of the generation max and population size

    // generate population_0
    // evaluate population_0
    // for (t = 0; t < maxGens; t++):
    //     Select population_t+1 from population_t
    //     NOTE: this is done for every single pair of individuals that we selected
    //         Recombine population_t+1
    //         Evaluate population_t+1
    //     do some bookkeeping

    // std::cout << std::fixed << std::setprecision(6);

    for (int i = 0; i < 30; i++)
    {
        Statistics stats = RunGeneticAlgorithm();
        std::stringstream filename;
        filename << "data/fitnessStatsRun" << i << ".csv";
        std::ofstream outFile(filename.str());
        OutputStatistics(stats, outFile);
    }

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
    for (Room& room : roomSet)
    {
        room.x = GenerateFloatInRange(generator, defaultRange);
        room.y = GenerateFloatInRange(generator, defaultRange);
    }

    x.chromosome = EncodeChromosome(roomSet);
    // PrintRoomSet(roomSet);
    // PrintChromosome(x.chromosome);

    static constexpr std::array<uint8_t, 3> livingColor{255, 0, 0};    // red
    static constexpr std::array<uint8_t, 3> kitchenColor{0, 255, 0};   // green
    static constexpr std::array<uint8_t, 3> bathColor{0, 0, 255};      // blue
    static constexpr std::array<uint8_t, 3> hallColor{128, 128, 128};  // gray
    static constexpr std::array<uint8_t, 3> bed1Color{255, 0, 255};    // purple
    static constexpr std::array<uint8_t, 3> bed2Color{0, 255, 255};    // cyan
    static constexpr std::array<uint8_t, 3> bed3Color{255, 255, 0};    // yellow
    static constexpr std::array colors = {livingColor, kitchenColor, bathColor, hallColor,
                                          bed1Color,   bed2Color,    bed3Color};

    // image layout is...
    // x 0 -> 1023 (length)
    // y 0
    // |
    // V 1023 (width)
    // cimg_library::CImg<uint8_t> image(1024, 1024, 1, 3, 0);
    // for (int i = 0; i < NUM_ROOMS; i++)
    // {
    //     Room& room = roomSet[i];
    //     const int xRoot = static_cast<int>(room.x * 10.0f);
    //     const int yRoot = static_cast<int>(room.y * 10.0f);
    //     const int length = static_cast<int>(room.length * 10.0f);
    //     const int width = static_cast<int>(room.width * 10.0f);

    //     const int xMax = xRoot + length > 1023 ? 1023 : xRoot + length;
    //     const int yMax = yRoot + width > 1023 ? 1023 : yRoot + width;

    //     for (int x = xRoot; x < xMax; x++)
    //     {
    //         for (int y = yRoot; y < yMax; y++)
    //         {
    //             image(x, y, 0, 0) = colors[i][0];
    //             image(x, y, 0, 1) = colors[i][1];
    //             image(x, y, 0, 2) = colors[i][2];
    //         }
    //     }
    // }
    // image.save_jpeg("out.jpeg");

    // std::this_thread::sleep_for(std::chrono::seconds(1));
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
