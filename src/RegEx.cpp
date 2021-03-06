#include "RegEx.h"
#include <cassert>

RegEx::RegEx(std::string inPattern) {
	pattern = inPattern;
	construct();
	deperenthesize();
}

void RegEx::construct() {
	std::vector<RegExState*> previousStates;
	std::vector<RegExState*> currentStates;
	std::stack<std::pair<std::vector<RegExState*>, std::vector<RegExState*> > > perenStack;
	bool alternating = false;
	begin = new RegExState();
	currentStates.push_back(begin);
	for (int i = 0; i < pattern.length(); i++) {
		switch (pattern[i]) {
			case '*':
			{
				//std::cout << "Star at " << i << " in " << pattern << std::endl;
				// for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++)
				// 	for (std::vector<RegExState*>::size_type k = 0; k < currentStates.size(); k++)
				// 		currentStates[j]->addNext(currentStates[k]);
				currentStates[currentStates.size()-1]->addNext(currentStates[currentStates.size()-1]);
				//add all previous states to current states to enable skipping over the starred item
				currentStates.insert(currentStates.end(), previousStates.begin(), previousStates.end());
			}
				break;
			case '+':
			{
				//std::cout << "Plus at " << i << " in " << pattern << std::endl;
				//OtherThingy
				//current->addNext(current);
				// for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++)
				// 	for (std::vector<RegExState*>::size_type k = 0; k < currentStates.size(); k++)
				// 		currentStates[j]->addNext(currentStates[k]);
				currentStates[currentStates.size()-1]->addNext(currentStates[currentStates.size()-1]);
			}
				break;
			case '?':
			{
				//std::cout << "Question at " << i << " in " << pattern << std::endl;
				//add all previous states to current states to enable skipping over the questioned item
				currentStates.insert(currentStates.end(), previousStates.begin(), previousStates.end());
			}
				break;
			case '|':
			{
				//std::cout << "Alternation at " << i << " in " << pattern << std::endl;
				//alternation
				alternating = true;
			}

				break;
			case '(':
			{
				//std::cout << "Begin peren at " << i << " in " << pattern << std::endl;
				//perentheses
				//Create a peren node with an inner empty node
				RegExState* next = new RegExState(new RegExState());

				if (alternating) {
					for (std::vector<RegExState*>::size_type j = 0; j < previousStates.size(); j++)
						previousStates[j]->addNext(next);

					//Save both current states here as well as the current preren
					std::vector<RegExState*> savePreviousStates = previousStates;
					currentStates.push_back(next);
					std::vector<RegExState*> saveCurrentStates = currentStates;
					perenStack.push(std::make_pair(savePreviousStates, saveCurrentStates));

					previousStates.clear();
					currentStates.clear();
					currentStates.push_back(next->getInner());
					alternating = false;
				} else {
					for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++)
						currentStates[j]->addNext(next);

					//Save both current states here as well as the current preren
					std::vector<RegExState*> savePreviousStates = currentStates;
					currentStates.clear();
					currentStates.push_back(next);
					std::vector<RegExState*> saveCurrentStates = currentStates;
					perenStack.push(std::make_pair(savePreviousStates, saveCurrentStates));

					previousStates.clear();
					currentStates.clear();
					currentStates.push_back(next->getInner());
				}
				//std::cout << "Peren is " << next << " Inner is " << currentStates[0] << " = " << next->getInner() << std::endl;
			}
				break;

			case ')':
			{
				//std::cout << "End peren at " << i << " in " << pattern << std::endl;
				//perentheses
				//Pop off the states that will now be the previous states and the peren node which will now be the current node
				std::pair<std::vector<RegExState*>, std::vector<RegExState*> > savedPair = perenStack.top();
				perenStack.pop();
				//Make the it so
				previousStates = savedPair.first;
				//Make sure the end of the inner stuff points back to the peren node
				for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++)
					currentStates[j]->addNext(savedPair.second[savedPair.second.size()-1]);
					//currentStates[j]->addNext(*(savedPair.second.end()));
				currentStates.clear();
				currentStates = savedPair.second;
			}
				break;

			case '\\':
			{
				i++;
				//std::cout << "Escape! Escaping: " << pattern[i] << std::endl;
				//Ahh, it's escaping a special character, so fall through to the default.
			}
			default:
			{
				//std::cout << "Regular" << std::endl;
				//Ahh, it's regular
				RegExState* next = new RegExState(pattern[i]);
				//If we're alternating, add next as the next for each previous state, and add self to currentStates
				if (alternating) {
					for (std::vector<RegExState*>::size_type j = 0; j < previousStates.size(); j++) {
						previousStates[j]->addNext(next);
						//std::cout << "Adding " << next << ", which is " << pattern[i] << " to " << previousStates[j] << std::endl;
					}
					currentStates.push_back(next);
					alternating = false;
				} else {
					//If we're not alternating, add next as next for all the current states, make the current states the new
					//previous states, and add ourself as the new current state.
					for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++) {
						currentStates[j]->addNext(next);
						//std::cout << "Adding " << next << ", which is " << pattern[i] << " to " << currentStates[j] << std::endl;
					}
					previousStates.clear();
					previousStates = currentStates;
					currentStates.clear();
					currentStates.push_back(next);
				}
			}
		}
	}
	//last one is goal state
	for (std::vector<RegExState*>::size_type i = 0; i < currentStates.size(); i++)
		currentStates[i]->addNext(NULL);
}

void RegEx::deperenthesize() {
	//std::cout << "About to de-perenthesize " << begin->toString() << std::endl;

	//Now go through and expand the peren nodes to regular nodes
	std::vector<RegExState*> processedStates;
	std::vector<RegExState*> statesToProcess;
	statesToProcess.push_back(begin);
	for (std::vector<RegExState*>::size_type i = 0; i < statesToProcess.size(); i++) {
		//Don't process null (sucess) state
		if (statesToProcess[i] == NULL)
			continue;
		std::vector<RegExState*>* nextStates = statesToProcess[i]->getNextStates();
		for (std::vector<RegExState*>::size_type j = 0; j < nextStates->size(); j++) {
			if ((*nextStates)[j] != NULL && (*nextStates)[j]->getInner() != NULL) {
				//Fix all the next references pointing to the peren node to point to the inner nodes. (if more than one, push back to add others)
				std::vector<RegExState*>* insideNextStates = (*nextStates)[j]->getInner()->getNextStates();
				//std::cout << "insideNextStates = " << insideNextStates << " [0] " << (*insideNextStates)[0] << std::endl;
				RegExState* perenState = (*nextStates)[j];
				(*nextStates)[j] = (*insideNextStates)[0];
				//std::cout << "So now nextstates[j] = " << (*nextStates)[j] << std::endl;
				for (std::vector<RegExState*>::size_type k = 1; k < insideNextStates->size(); k++)
					nextStates->push_back((*insideNextStates)[k]);
				//std::cout << "Replaced beginning: " << begin->toString() << std::endl;
				//Now, if the peren node is self-referential (has a repitition operator after i), fix it's self-references in the same manner
				std::vector<RegExState*>* perenNextNodes = perenState->getNextStates();
				for (std::vector<RegExState*>::size_type k = 0; k < perenNextNodes->size(); k++) {
					if ((*perenNextNodes)[k] == perenState) {
						(*perenNextNodes)[k] = (*insideNextStates)[0];
						for (std::vector<RegExState*>::size_type l = 1; l < insideNextStates->size(); l++)
							perenNextNodes->push_back((*insideNextStates)[l]);
					}
				}
				//std::cout << "Fixed self-references: " << begin->toString() << std::endl;
				//Need to fix the end too
				std::vector<RegExState*> traversalList;
				traversalList.push_back(perenState->getInner());
				for (std::vector<RegExState*>::size_type k = 0; k < traversalList.size(); k++) {
					std::vector<RegExState*>* nextTraversalStates = traversalList[k]->getNextStates();
					//std::cout << "Traversing! nextTraversalStates from traversalList " << traversalList[k]  << " char = " << traversalList[k]->getCharacter() << std::endl;
					//std::cout << "with children:" << std::endl;
					//for (std::vector<RegExState*>::size_type l = 0; l < nextTraversalStates->size(); l++)
					//	std::cout << "\t\"" << (*nextTraversalStates)[l]->getCharacter() << "\"" << std::endl;
					//std::cout << std::endl; 
					for (std::vector<RegExState*>::size_type l = 0; l < nextTraversalStates->size(); l++) {
						//If this node is equal to the peren node we came from, then that means we've reached the end of the inner part of the peren
						//And we now replace this reference with the next nodes from the peren node
						//std::cout << "Traversal Next is on " << (*nextTraversalStates)[l]->getCharacter() << std::endl;
						if ((*nextTraversalStates)[l] == perenState) {
						//	std::cout << "nextTraversalStates[l] = to perenState!" << std::endl;
							std::vector<RegExState*> endPerenNextStates = *(perenState->getNextStates());
							(*nextTraversalStates)[l] = endPerenNextStates[0];
							for (std::vector<RegExState*>::size_type n = 1; n < endPerenNextStates.size(); n++)
								nextTraversalStates->push_back(endPerenNextStates[n]);
							//Now make sure we don't now try to continue through and end up processing stuff we just replaced the peren reference with
							break;
						} else {
							traversalList.push_back((*nextTraversalStates)[l]);
						}
					}
				}
			}
		}
		//Now add all these next states to process, only if they haven't already been processed
		for (std::vector<RegExState*>::size_type j = 0; j < nextStates->size(); j++) {
			bool inCurrStates = false;
			for (std::vector<RegExState*>::size_type k = 0; k < statesToProcess.size(); k++) {
				if ((*nextStates)[j] == statesToProcess[k])
					inCurrStates = true;
			}
			if (!inCurrStates) {
				statesToProcess.push_back((*nextStates)[j]);
				//std::cout << (*nextStates)[j] << "Is not in states to process" << std::endl;
			}
		}
	}
	//std::cout << "Finished de-perenthesization " << begin->toString() << std::endl;
}

RegEx::~RegEx() {
	//No cleanup necessary
}

int RegEx::longMatch(std::string stringToMatch) {
	// Start in the begin state (only).
	int lastMatch = -1;
	currentStates.clear();
	currentStates.push_back(begin);
	std::vector<RegExState*> nextStates;

	for (int i = 0; i < stringToMatch.size(); i++) {
		//Go through every current state. Check to see if it is goal, if so update last goal.
		//Also, add each state's advance to nextStates
		for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++) {
			if (currentStates[j]->isGoal()) {
				lastMatch = i;
				//std::cout << "Hit goal at " << i << " character: " << stringToMatch[i-1] << std::endl;
			} else {
				//std::cout << "currentState " << j << ", " << currentStates[j]->toString() << " is not goal" <<std::endl;
			}
			std::vector<RegExState*>* addStates = currentStates[j]->advance(stringToMatch.at(i));
			nextStates.insert(nextStates.end(), addStates->begin(), addStates->end());
			delete addStates;
		}
		//Now, clear our current states and add eaczh one of our addStates if it is not already in current states

		currentStates.clear();
		for (std::vector<RegExState*>::size_type j = 0; j < nextStates.size(); j++) {
			bool inCurrStates = false;
			for (std::vector<RegExState*>::size_type k = 0; k < currentStates.size(); k++) {
				if (nextStates[j] == currentStates[k])
					inCurrStates = true;
			}
			if (!inCurrStates)
				currentStates.push_back(nextStates[j]);
		}
		// if (currentStates.size() != 0)
		// 	std::cout << "Matched " << i << " character: " << stringToMatch[i-1] << std::endl;

		nextStates.clear();
		//If we can't continue matching, just return our last matched
		if (currentStates.size() == 0)
			break;
	}
	//Check to see if we match on the last character in the string
	for (std::vector<RegExState*>::size_type j = 0; j < currentStates.size(); j++) {
		if (currentStates[j]->isGoal())
			lastMatch = stringToMatch.size();
	}
	return lastMatch;
}

std::string RegEx::getPattern() {
	return pattern;
}

std::string RegEx::toString() {
	return pattern + " -> " + begin->toString();
}

void RegEx::test() {
    {
        RegEx re("a*");
        assert(re.longMatch("a") == 1);
        assert(re.longMatch("aa") == 2);
        assert(re.longMatch("aaaab") == 4);
        assert(re.longMatch("b") == 0);
    }

    {
        RegEx re("a+");
        assert(re.longMatch("aa") == 2);
        assert(re.longMatch("aaaab") == 4);
        assert(re.longMatch("b") == -1);
    }

    {
        RegEx re("a(bc)?");
        assert(re.longMatch("ab") == 1);
    }

    std::cout << "RegEx tests pass\n";
}
