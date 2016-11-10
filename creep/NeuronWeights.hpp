#ifndef CREEP_NEURONWEIGHTS_HPP
#define CREEP_NEURONWEIGHTS_HPP

#include <vector>
#include <memory>

typedef float Weight;
typedef std::vector<Weight> Weights;

// do not use serialization here. It makes little sense to save a shared
// pointer and it would break save compatibility
template<class Archive>
std::shared_ptr<Weights> loadWeights(Archive & ar)
{
    auto result = std::make_shared<Weights>();
    ar >> *result;
    return result;
}

#endif // CREEP_NEURONWEIGHTS_HPP
