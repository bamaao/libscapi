//
// Created by moriya on 04/01/17.
//

#ifndef GMW_GMWPARTY_H
#define GMW_GMWPARTY_H

#include "Circuit.h"
#include "MPCCommunication.h"
#include "../../include/primitives/Prg.hpp"
#include "../../include/CryptoInfra/Protocol.hpp"
#include "../../include/CryptoInfra/SecurityLevel.hpp"
#include "CBitVector.h"
#include <thread>
#include <mutex>

/**
 * This class represents the GMW protocol.
 * A general explanation of the GMW protocol can be found at http://crypto.biu.ac.il/sites/default/files/Winter%20School%2015%20-%20GMW%20and%20OT%20extension.pdf.
 * This implementation is more efficient since we use Beaver's multiplication triples instead of 1 out of 4 OT.
 */
class GMWParty : public Protocol, public SemiHonest{

private:
    boost::asio::io_service io_service;
    int id, numThreads, numPartiesForEachThread;
    shared_ptr<Circuit> circuit;
    vector<shared_ptr<ProtocolPartyData>> parties;
    vector<byte> aArray, bArray, cArray;
    vector<byte> wiresValues;
    mutex mtx;
    string inputFileName;
    vector<byte> output;
	vector<byte> myInputBits;

	/*
	 * Generates Beaver's multiplication triples to use in the protocol.
	 */
    void generateTriples();

	/*
	 * Executes the input sharing phase of the protocol. 
	 */
    void inputSharing();

	/*
	 * Computes the circuit.
	 */
    vector<byte>& computeCircuit();

	/*
	 * Reads the input from the given file.
	 */
    void readInputs(vector<byte> & inputs) const;

	/*
	 * This function generates the Beaver's triples for the parties listed by first and last parameters.
	 * The splitting to parties is done in order to enable execution using threads.
	 */
    void generateTriplesForParty(PrgFromOpenSSLAES & prg, int first, int last);

	/*
	* This function sends the generated input shares to the parties listed by first and last parameters.
	* The splitting to parties is done in order to enable execution using threads.
	*/
    void sendSharesToParties(PrgFromOpenSSLAES & prg, vector<byte> & myInputBits, int first, int last);

	/*
	 * Receive the shares from the party with index i.
	 */
    void receiveShares(vector<int> &otherInputWires, vector<byte> &otherShares, int i);

	/*
	* Compute the and gates of the circuit's level.
	*/
    void recomputeAndGatesWithThreads(int & firstAndGateToRecompute, vector<CBitVector> & myD,
                                      vector<CBitVector> & myE, int i, vector<bool> & isWireReady,
                                      int & numAndGatesComputed, int & andGatesComputedCounter);

	/*
	* This function Computes the and gates of the circuit's level by connecting to the parties listed by first and last parameters.
	* The splitting to parties is done in order to enable execution using threads.
	*/
    void recomputeAndGates(int firstAndGateToRecompute, vector<CBitVector> & myD, vector<CBitVector> & myE, int i,
                           vector<bool> & isWireReady, int numAndGatesComputed, int & andGatesComputedCounter, int first, int last);

	/*
	 * This function sends and receive the output shares from the other parties.
	 */
    vector<byte>& revealOutput();

	/*
	 * Send and receive the output shares from the parties listed by first and last parameters.
	* The splitting to parties is done in order to enable execution using threads.
	 */
    void revealOutputFromParty(vector<byte> & output, int first, int last);

public:

    GMWParty(int id, const shared_ptr<Circuit> & circuit, string partiesFileName, int numThreads, string inputFileName);

	~GMWParty() {
		io_service.stop();
	}

    /*
     * Implement the function derived from the Protocol abstract class.
     */
    void run() override;

	/*
	 * Executes the offline phase of the protocol.
	 */
    void runOffline();

	/*
	* Executes the online phase of the protocol.
	*/
    vector<byte>& runOnline();

    vector<shared_ptr<ProtocolPartyData>> & getParties(){ return parties; }


};
#endif //GMW_GMWPARTY_H
