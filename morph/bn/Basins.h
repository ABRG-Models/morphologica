/*
 * Basins of attraction code. For analysing the basins of attraction in a Genome or
 * GeneNet.
 *
 * Adapted from basins.h in AttractorScaffolding.
 *
 * Author: Seb James
 * Date: November 2020
 */
#pragma once

#include <map>
#include <set>
#include <vector>

namespace morph {
    namespace bn {

        //! An enumerated type for the end point of a cycle. Used in fitness functions.
        enum class endpoint
        {
            unknown,
            limit,
            point,
            num
        };

        /*!
         * When working with states in a graph of nodes, it may be necessary to use one
         * bit to refer to the state as being unset; this is the bit to use. This
         * precludes the use of N_Genes==8.
         */
        static constexpr state_t state_t_unset = 0x80;

        /*!
         * To make a graph of states, we need a state node object which has one child
         * node to which it transfers, but potentially many parent nodes. If parents set
         * is empty, then this StateNode is a starting state in a basin of attraction.
         */
        struct StateNode
        {
            StateNode(state_t s) : id(s) {}
            //! The identifier of this base node - its value. This is used as the key in
            //! maps of StateNodes.
            state_t id;
            //! The parents of the base node, which feed into it.
            set<state_t> parents;
            //! the child StateNode
            state_t child;
        };

        //! A container to hold the information about a single basin of attraction.
        struct BasinOfAttraction
        {
            /*!
             * Merge the other basin of attraction into this basin of attraction. The
             * assumption is that the limitCycle and endpoint of other match the
             * limitCycle and endpoint of this (I'm not going to waste compute cycles
             * comparing, as this will already have been done).
             */
            void merge (const BasinOfAttraction& other)
            {
                this->flags |= other.flags;
                // merge other.nodes into this->nodes
                typename std::map<state_t, StateNode>::iterator mi = this->nodes.begin();
                // Add parents from other states to parents of this
                while (mi != this->nodes.end()) {
                    state_t key = mi->first;
                    if (other.nodes.count(key)) {
                        mi->second.parents.insert (other.nodes.at(key).parents.begin(),
                                                   other.nodes.at(key).parents.end());
                    }
                    ++mi;
                }
                // THEN add any states in other.nodes that don't exist in this.
                map<state_t, StateNode>::const_iterator cmi = other.nodes.begin();
                while (cmi != other.nodes.end()) {
                    // cmi->first is the state_t id of a node in the other basin of
                    // attraction.
                    if (this->nodes.count (cmi->first) == 0) {
                        this->nodes.insert (make_pair(cmi->first, cmi->second));
                    }
                    ++cmi;
                }
                //DBGF ("After merge, this->nodes.size() = " << this->nodes.size());
            }

            //! An "output for debugging" method
            void debug() const
            {
                std::cout << "----------------------Basin-output-begin---------------------------" << std::endl;
                std::cout << "Basin of attraction with the attractor:" << std::endl;
                std::set<state_t>::const_iterator si = this->limitCycle.begin();
                while (si != this->limitCycle.end()) {
                    std::cout << "  " << state_str (*si) << std::endl;
                    si++;
                }
                std::cout << "Branches:" << std::endl;
                map<state_t, StateNode>::const_iterator mi = this->nodes.begin();
                while (mi != this->nodes.end()) {
                    if (mi->second.parents.empty()) {
                        // Then this is an "outer node" on the basin. Show
                        // its progress to the attractor
                        state_t state = mi->first;
                        StateNode sn = mi->second;
                        while (this->limitCycle.count(state) == 0) {
                            std::cout << " --> " << state_str (state) << "(" << (unsigned int)state << ")";
                            state = sn.child;
                            sn = this->nodes.at (state);
                        }
                        set<state_t>::const_iterator si = this->limitCycle.begin();
                        std::cout << " -->* ";
                        while (si != this->limitCycle.end()) {
                            std::cout << state_str (*si) << "("<< (unsigned int)state << "):";
                            ++si;
                        }
                        std::cout << std::endl;

                    }
                    ++mi;
                }
                std::cout << "Nodes in basin:" << this->nodes.size() << std::endl;
                std::cout << "Transitions in basin:" << std::endl;
                mi = this->nodes.begin();
                while (mi != this->nodes.end()) {
                    std::cout << state_str(mi->second.id) << " --> " << state_str(mi->second.child) << std::endl;
                    ++mi;
                }
                std::cout << "-----------------------Basin-output-end----------------------------" << std::endl;
            }

            /*!
             * Return the set of state to state transitions in this basin of attraction.
             */
            std::set<unsigned int> getTransitionSet() const
            {
                std::set<unsigned int> transitions;
                std::map<state_t, StateNode>::const_iterator mi = this->nodes.begin();
                while (mi != this->nodes.end()) {
                    unsigned int trans = ((unsigned int)mi->second.id << 16) | (unsigned int)mi->second.child;
                    //DBG2 ("Inserting transition 0x" << hex << trans << dec);
                    transitions.insert (trans);
                    ++mi;
                }
                return transitions;
            }

            //! Is the endpoint type a fixed attractor or a limit cycle?
            morph::bn::endpoint endpoint = endpoint::unknown;

            /*!
             * The set of states in the limit cycle. Will be a set of size 1 if endpoint
             * is a fixed point attractor. This is used to determine if one partially
             * determined basin of attraction matches another, determined basin of
             * attraction.
             */
            std::set<state_t> limitCycle;

            //! The full basin of attraction.
            std::map<state_t, StateNode> nodes;
        };

        /*!
         * Container class to hold several basins of attraction for a particular genome
         * and some information that can be obtained from them.
         */
        template <size_t N=5, size_t K=N>
        struct AllBasins
        {
            AllBasins (const Genome<N,K>& g)
            {
                this->update (g);
            }

            void update (const Genome<N,K>& g)
            {
                this->genome = g;
                this->basins.clear();
                this->attractorSizes.clear();
                this->transitions.clear();
                find_basins_of_attraction (this->genome, this->basins);
                std::vector<BasinOfAttraction>::const_iterator i = this->basins.begin();
                while (i != basins.end()) {
                    std::set<unsigned int> tset = i->getTransitionSet();
                    this->transitions.insert(tset.begin(), tset.end());
                    this->attractorSizes.push_back(i->limitCycle.size());
                    ++i;
                }
            }

            //! Find all the basins of attraction for the given genome.
            void find_basins_of_attraction()
            {
                for (state_t s = 0; s < (1<<N); ++s) {

                    // First check if s is in any of the basins we already computed.
                    bool s_encountered = false;
                    typename std::vector<BasinOfAttraction>::iterator bi = this->basins.begin();
                    while (bi != this->basins.end()) {
                        if (bi->nodes.count(s) > 0) {
                            s_encountered = true;
                            break;
                        }
                        ++bi;
                    }
                    if (s_encountered == true) {
                        continue; // on to next s
                    }

                    // A new basin of attraction that we'll find.
                    BasinOfAttraction basin;
                    // A copy of starting point s.
                    state_t st = s;
                    state_t next_st = state_t_unset;
                    state_t last_st = state_t_unset;

                    unsigned int saw_flags = 0x0;

                    for (;;) {
                        // For the current state, st, compute what the next state will be.
                        next_st = st;
                        // Becomes.. GeneNet.develop()
                        compute_next (genome, next_st);

                        // Create state node
                        StateNode stnode(st); // node for current state.
                        stnode.child = next_st; // Mark the child state
                        if (last_st != state_t_unset) {
                            stnode.parents.insert (last_st);
                        }

                        if (basin.nodes.count (st) > 0) {
                            // Already visited this state so it's in an attractor
                            //DBG2 ("Repeat st " << state_str(st) << "!");
                            if (st == last_st) {
                                // It's a point attractor
                                //DBGF("fixed point attractor");
                                basin.endpoint = endpoint::point;
                                // Which means it's its own parent. Set parent of the node that's ALREADY BEEN added to basin.
                                basin.nodes.find(st)->second.parents.insert (st);
                                basin.limitCycle.insert (st);
                            } else {
                                // It's a limit cycle.
                                //DBGF("limit cycle attractor");
                                // Go around the limit cycle to determine its identity, which is a set of states.
                                basin.endpoint = endpoint::limit;
                                // Update the parent of the copy of this state that's already in the basin object
                                basin.nodes.find(st)->second.parents.insert (last_st);

                                // Cycle around filling the limit cycle set in basin.limitCycle
                                state_t lc = st;
                                for (;;) {
                                    basin.limitCycle.insert (lc);
                                    state_t c = basin.nodes.find (lc)->second.child;
                                    if (c == st) {
                                        // We're back to the start
                                        break;
                                    } else {
                                        lc = c;
                                    }
                                }
                            }

                            break;
                        }

                        // Insert it into basin
                        basin.nodes.insert (make_pair (st, stnode));
                        //DBG2 ("Inserting " << state_str (st));

                        // Update last_st and st
                        last_st = st;
                        st = next_st;
                    }

                    // Now see if our basin is already present in basins, and if not, simply push_back.
                    bool found = false;
                    bi = thish->basins.begin();
                    while (bi != this->basins.end()) {
                        // Nice thing with sets is that we can directly compare them.
                        if (basin.limitCycle == bi->limitCycle) {
                            // Merge basin into bi->second, because this limitCycle was already found.
                            bi->merge (basin);
                            found = true;
                            break;
                        }
                        ++bi;
                    }
                    if (!found) {
                       this->basins.push_back (basin);
                    }
                }
            }

            //! The genome to be analysed (might be better as GeneNet.
            Genome<N,K> genome;

            //! All the basins of attraction. Could this become a member of GeneNet? A
            //! GeneNet net with a given Genome has a defined number of basins of
            //! attraction.
            std::vector<BasinOfAttraction> basins;

            unsigned int getNumBasins() { return this->basins.size(); }

            //! Holds a list of the sizes of the attractor limit cycles.
            std::vector<unsigned int> attractorSizes;

            double meanAttractorLength()
            {
                unsigned int sum = 0.0;
                for (unsigned int i : this->attractorSizes) {
                    sum += i;
                }
                return (static_cast<double>(sum)/static_cast<double>(attractorSizes.size()));
            }

            unsigned int maxAttractorLength()
            {
                unsigned int max = 0;
                for (unsigned int i : this->attractorSizes) {
                    max = i > max ? i : max;
                }
                return max;
            }

            //! Return the basin of attraction which contains the state st.
            BasinOfAttraction find (state_t& st)
            {
                for (auto b : this->basins) {
                    try {
                        StateNode sn = b.nodes.at(st);
                        return b;
                    } catch (const std::out_of_range& e) {
                        // st in not in b.
                    }
                }
                BasinOfAttraction nullbasin;
                return nullbasin;
            }

            /*!
             * Holds all the transitions in all the basins. Should have size exactly
             * 2^N_Genes. Will break for N_Genes > 16 (we store two genes in each
             * unsigned int; 2*16=32).
             */
            std::set<unsigned int> transitions;
        };

    } // namespace bn
} // namespace morph
