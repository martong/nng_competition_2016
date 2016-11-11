#ifndef GENOME_HPP
#define GENOME_HPP

#include <vector>
#include <memory>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "NeuronWeights.hpp"

class Genome {
public:
    Genome() = default;
    Genome(Weights weights, float fitness = 0.f) :
        weights(std::move(weights)), fitness(fitness) {}

    Weights weights;
    float fitness = 0.f;
    std::string debugInfo;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void load(Archive& ar, const unsigned /*version*/) {
        ar >> weights;
    }

    template<class Archive>
    void save(Archive& ar, const unsigned /*version*/) const {
        ar << weights;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

typedef std::vector<Genome> Genomes;

inline
bool operator<(const Genome& left, const Genome& right) {
    return left.fitness < right.fitness;
}

BOOST_CLASS_VERSION(Genome, 0)

#endif /* !GENOME_HPP */
