/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information
 * -------------------------------------------------------------------------- */

/**
 * @file    BayesTreeCliqueBase-inst.h
 * @brief   Base class for cliques of a BayesTree
 * @author  Richard Roberts and Frank Dellaert
 */

#pragma once

#include <gtsam/inference/BayesTreeCliqueBaseUnordered.h>
#include <boost/foreach.hpp>

namespace gtsam {

  /* ************************************************************************* */
  template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  bool BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::equals(
    const DERIVED& other, double tol = 1e-9) const
  {
    return (!conditional_ && !other.conditional())
      || conditional_->equals(*other.conditional(), tol);
  }

  ///* ************************************************************************* */
  //template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  //std::vector<Key>
  //  BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::separator_setminus_B(derived_ptr B) const
  //{
  //  FastSet<Key> p_F_S_parents(this->conditional()->beginParents(), this->conditional()->endParents());
  //  FastSet<Key> indicesB(B->conditional()->begin(), B->conditional()->end());
  //  std::vector<Key> S_setminus_B;
  //  std::set_difference(p_F_S_parents.begin(), p_F_S_parents.end(),
  //    indicesB.begin(), indicesB.end(), back_inserter(S_setminus_B));
  //  return S_setminus_B;
  //}

  ///* ************************************************************************* */
  //template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  //std::vector<Key> BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::shortcut_indices(
  //  derived_ptr B, const FactorGraphType& p_Cp_B) const
  //{
  //  gttic(shortcut_indices);
  //  FastSet<Key> allKeys = p_Cp_B.keys();
  //  FastSet<Key> indicesB(B->conditional()->begin(), B->conditional()->end());
  //  std::vector<Key> S_setminus_B = separator_setminus_B(B);
  //  std::vector<Key> keep;
  //  // keep = S\B intersect allKeys (S_setminus_B is already sorted)
  //  std::set_intersection(S_setminus_B.begin(), S_setminus_B.end(), //
  //    allKeys.begin(), allKeys.end(), back_inserter(keep));
  //  // keep += B intersect allKeys
  //  std::set_intersection(indicesB.begin(), indicesB.end(), //
  //    allKeys.begin(), allKeys.end(), back_inserter(keep));
  //  return keep;
  //}

  /* ************************************************************************* */
  template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  void BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::print(
    const std::string& s, const KeyFormatter& keyFormatter) const
  {
    conditional_->print(s, keyFormatter);
  }

  /* ************************************************************************* */
  template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  size_t BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::treeSize() const {
    size_t size = 1;
    BOOST_FOREACH(const derived_ptr& child, children)
      size += child->treeSize();
    return size;
  }

  /* ************************************************************************* */
  template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  size_t BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::numCachedSeparatorMarginals() const
  {
    if (!cachedSeparatorMarginal_)
      return 0;

    size_t subtree_count = 1;
    BOOST_FOREACH(const derived_ptr& child, children)
      subtree_count += child->numCachedSeparatorMarginals();

    return subtree_count;
  }

  /* ************************************************************************* */
  // The shortcut density is a conditional P(S|R) of the separator of this
  // clique on the root. We can compute it recursively from the parent shortcut
  // P(Sp|R) as \int P(Fp|Sp) P(Sp|R), where Fp are the frontal nodes in p
  /* ************************************************************************* */
  //template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  //BayesNetUnordered<CONDITIONAL> BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::shortcut(
  //  derived_ptr B, Eliminate function) const
  //{
  //  gttic(BayesTreeCliqueBaseUnordered_shortcut);

  //  // We only calculate the shortcut when this clique is not B
  //  // and when the S\B is not empty
  //  std::vector<Key> S_setminus_B = separator_setminus_B(B);
  //  if (B.get() != this && !S_setminus_B.empty()) {

  //    // Obtain P(Cp||B) = P(Fp|Sp) * P(Sp||B) as a factor graph
  //    derived_ptr parent(parent_.lock());
  //    gttoc(BayesTreeCliqueBaseUnordered_shortcut);
  //    FactorGraphType p_Cp_B(parent->shortcut(B, function)); // P(Sp||B)
  //    gttic(BayesTreeCliqueBaseUnordered_shortcut);
  //    p_Cp_B.push_back(parent->conditional()->toFactor()); // P(Fp|Sp)

  //    // Determine the variables we want to keepSet, S union B
  //    std::vector<Key> keep = shortcut_indices(B, p_Cp_B);

  //    // Reduce the variable indices to start at zero
  //    gttic(Reduce);
  //    const Permutation reduction = internal::createReducingPermutation(p_Cp_B.keys());
  //    internal::Reduction inverseReduction = internal::Reduction::CreateAsInverse(reduction);
  //    BOOST_FOREACH(const boost::shared_ptr<FactorType>& factor, p_Cp_B) {
  //      if(factor) factor->reduceWithInverse(inverseReduction); }
  //    inverseReduction.applyInverse(keep);
  //    gttoc(Reduce);

  //    // Create solver that will marginalize for us
  //    GenericSequentialSolver<FactorType> solver(p_Cp_B);

  //    // Finally, we only want to have S\B variables in the Bayes net, so
  //    size_t nrFrontals = S_setminus_B.size();
  //    BayesNet<CONDITIONAL> result = *solver.conditionalBayesNet(keep, nrFrontals, function);

  //    // Undo the reduction
  //    gttic(Undo_Reduce);
  //    BOOST_FOREACH(const typename boost::shared_ptr<FactorType>& factor, p_Cp_B) {
  //      if (factor) factor->permuteWithInverse(reduction); }
  //    result.permuteWithInverse(reduction);
  //    gttoc(Undo_Reduce);

  //    assertInvariants();

  //    return result;
  //  } else {
  //    return BayesNet<CONDITIONAL>();
  //  }
  //}

  /* ************************************************************************* */
  // separator marginal, uses separator marginal of parent recursively
  // P(C) = P(F|S) P(S)
  /* ************************************************************************* */
  //template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  //FactorGraph<typename BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::FactorType> BayesTreeCliqueBaseUnordered<
  //    DERIVED, CONDITIONAL>::separatorMarginal(derived_ptr R, Eliminate function) const
  //{
  //  gttic(BayesTreeCliqueBaseUnordered_separatorMarginal);
  //  // Check if the Separator marginal was already calculated
  //  if (!cachedSeparatorMarginal_) {
  //    gttic(BayesTreeCliqueBaseUnordered_separatorMarginal_cachemiss);
  //    // If this is the root, there is no separator
  //    if (R.get() == this) {
  //      // we are root, return empty
  //      FactorGraph<FactorType> empty;
  //      cachedSeparatorMarginal_ = empty;
  //    } else {
  //      // Obtain P(S) = \int P(Cp) = \int P(Fp|Sp) P(Sp)
  //      // initialize P(Cp) with the parent separator marginal
  //      derived_ptr parent(parent_.lock());
  //      gttoc(BayesTreeCliqueBaseUnordered_separatorMarginal_cachemiss); // Flatten recursion in timing outline
  //      gttoc(BayesTreeCliqueBaseUnordered_separatorMarginal);
  //      FactorGraph<FactorType> p_Cp(parent->separatorMarginal(R, function)); // P(Sp)
  //      gttic(BayesTreeCliqueBaseUnordered_separatorMarginal);
  //      gttic(BayesTreeCliqueBaseUnordered_separatorMarginal_cachemiss);
  //      // now add the parent conditional
  //      p_Cp.push_back(parent->conditional()->toFactor()); // P(Fp|Sp)

  //      // Reduce the variable indices to start at zero
  //      gttic(Reduce);
  //      const Permutation reduction = internal::createReducingPermutation(p_Cp.keys());
  //      internal::Reduction inverseReduction = internal::Reduction::CreateAsInverse(reduction);
  //      BOOST_FOREACH(const boost::shared_ptr<FactorType>& factor, p_Cp) {
  //        if(factor) factor->reduceWithInverse(inverseReduction); }

  //      // The variables we want to keepSet are exactly the ones in S
  //      sharedConditional p_F_S = this->conditional();
  //      std::vector<Key> indicesS(p_F_S->beginParents(), p_F_S->endParents());
  //      inverseReduction.applyInverse(indicesS);
  //      gttoc(Reduce);

  //      // Create solver that will marginalize for us
  //      GenericSequentialSolver<FactorType> solver(p_Cp);

  //      cachedSeparatorMarginal_ = *(solver.jointBayesNet(indicesS, function));

  //      // Undo the reduction
  //      gttic(Undo_Reduce);
  //      BOOST_FOREACH(const typename boost::shared_ptr<FactorType>& factor, p_Cp) {
  //        if (factor) factor->permuteWithInverse(reduction); }
  //      BOOST_FOREACH(const typename boost::shared_ptr<FactorType>& factor, *cachedSeparatorMarginal_) {
  //        if (factor) factor->permuteWithInverse(reduction); }
  //      gttoc(Undo_Reduce);
  //    }
  //  } else {
  //    gttic(BayesTreeCliqueBaseUnordered_separatorMarginal_cachehit);
  //  }

  //  // return the shortcut P(S||B)
  //  return *cachedSeparatorMarginal_; // return the cached version
  //}

  /* ************************************************************************* */
  // marginal2, uses separator marginal of parent recursively
  // P(C) = P(F|S) P(S)
  /* ************************************************************************* */
  //template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  //FactorGraph<typename BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::FactorType> BayesTreeCliqueBaseUnordered<
  //    DERIVED, CONDITIONAL>::marginal2(derived_ptr R, Eliminate function) const
  //{
  //  gttic(BayesTreeCliqueBaseUnordered_marginal2);
  //  // initialize with separator marginal P(S)
  //  FactorGraph<FactorType> p_C(this->separatorMarginal(R, function));
  //  // add the conditional P(F|S)
  //  p_C.push_back(this->conditional()->toFactor());
  //  return p_C;
  //}

  /* ************************************************************************* */
  template<class DERIVED, class FACTORGRAPH, class BAYESNET>
  void BayesTreeCliqueBaseUnordered<DERIVED, FACTORGRAPH, BAYESNET>::deleteCachedShortcuts() {

    // When a shortcut is requested, all of the shortcuts between it and the
    // root are also generated. So, if this clique's cached shortcut is set,
    // recursively call over all child cliques. Otherwise, it is unnecessary.
    if (cachedSeparatorMarginal_) {
      BOOST_FOREACH(derived_ptr& child, children) {
        child->deleteCachedShortcuts();
      }

      //Delete CachedShortcut for this clique
      cachedSeparatorMarginal_ = boost::none;
    }

  }

}
