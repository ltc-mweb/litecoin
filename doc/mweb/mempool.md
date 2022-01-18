# Mempool

### Accepting Txs

* For transactions with canonical tx data, we should only accept those whose kernels match the transaction's peg-ins.
  * Use standard tx/wtx hash for CInv/GetData messages.
  * Theoretically could still have malleable inputs/outputs, but we'll probably just always keep the first we see?
* For all other MWEB txs (those with no canonical tx data), we want to support multiple kernels.
  * Use kernel ID's for CInv/GetData messages.

* **AcceptSingleTransaction(const CTransactionRef&, ATMPArgs&)**
  1. PreChecks
     a. CheckTransaction
     b. No Coinbase or HogEx
     c. IsStandardTx
     d. CheckFinalTx
     e. Check if already in mempool
     f. Find conflicting txs and check if can RBF
     g. Verify HaveCoin is true for all inputs
     h. CheckSequenceLocks
     i. Consensus::CheckTxInputs
     j. AreInputsStandard and IsWitnessStandard
     k. Build CTxMemPoolEntry
     l. Check Fee Rate
  2. PolicyScriptChecks - calls CheckInputScripts
  3. ConsensusScriptChecks - calls CheckInputsFromMempoolAndCache
  4. Finalize
     1. Calls CTxMempool::RemoveStaged for conflicting transactions
     2. CTxMempool::addUnchecked
     3. LimitMempoolSize

### Data Structures

* **mapTx** (existing) - boost::multi_index containing the mempool txs sorted by txid, wtxid, descendant feerate, ancestor feerate, and time in mempool. 
* **mapNextTx** (existing) - Maps outputs to mempool transactions that spend them.
* **vTxHashes** (existing) - 
* **mapDeltas** (existing) - 
* **mapTxOutputs_MWEB** (new) - Maps MWEB output IDs to mempool transactions that create them.

### Mempool Functions

* **exists(const GenTxid&)**
* **info(const GenTxid&)**
* **get(const uint256&)**
* **AlreadyHaveTx(const GetTxid&)**
* **HasNoInputsOf(const CTransaction&)**
---

* **UpdateMempoolForReorg**