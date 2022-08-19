#include <stdexcept>      // std::out_of_range

template<class P, class N>
class Remap {
public:
  /**
   * @brief Construct a new Remap object
   * 
   * @param prevMin the previous range minimum value
   * @param prevMax the previous range maximum value
   * @param newMin the new range minimum value
   * @param newMax the new range maximum value
   */
  Remap(P prevMin, P prevMax, N newMin, N newMax) :
    prev_min_(prevMin),
    prev_max_(prevMax),
    new_min_(newMin),
    new_max_(newMax)
  {
    if (prev_min_ == prev_max_)
        throw std::out_of_range("prev_min_ == prev_max_ !!\n");
    scale_ = (0.5*new_max_ - 0.5*new_min_)/(0.5*prev_max_ - 0.5*prev_min_);
  }

  // Convert a previous value to the new mapping
  N Convert(P v)
  {
    return 2*static_cast<N>((0.5*v - 0.5*prev_min_) * scale_ + 0.5*new_min_);
  }

private:
  P prev_min_;
  P prev_max_;
  N new_min_;
  N new_max_;
  double scale_; // Replaces prev_range_ and new_range_
};
