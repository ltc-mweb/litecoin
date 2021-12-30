# MWEB Consensus

## Rules

#### Output Uniqueness

* Output commitments in the UTXO set shall be unique at all times.
* A block shall not be included when it contains an output commitment which already exists in the UTXO set.
  * Block is still invalid even if it also contains an input with said commitment.
* A block shall not contain 2 outputs with the same commitment.
  * Block is still invalid even if it also contains an input with said commitment.
* A block *could* contain an input which spends an output created in the same block, provided the commitment was not already in the UTXO set.
  * To better support payment proofs, the outputs shall still be added to the TXO MMR, but shall be marked as spent.
---

#### Kernel Uniqueness

* A block shall not contain 2 kernels with the same commitment.
---

#### Ordering

* Inputs in a block shall be in ascending order by their raw commitment value.
* Outputs in a block shall be in ascending order by their raw commitment value.
* Kernels in a block shall list pegins first, and then be ordered in ascending order by their raw hashed value.
* Signed owner messages shall be in ascending order by their raw hashed value.
---

#### PMMRs

* After building an MMR with all of the kernels from a block, the root shall match the header's kernel root.
* The number of kernels in a block shall match the header's kernel size.
* After adding all outputs from a block to the end of the output PMMR, the root shall match the header's output root.
* After adding all outputs from a block to the end of the output PMMR, the size shall match the header's output size.
---

#### UTXO LeafSet

* A simple bitset shall be created and maintained to keep track of which TXOs in the output PMMR remain unspent.
* The bit positions shall map 1-to-1 to to PMMR leaf indices. A `0` at that bit position means spent, whereas a `1` means unspent. 
  * Ex: If byte 0 bit 2 is a `1`, that means TXO at leaf index `2` is unspent. If byte 2 bit 1 is a `0`, the TXO at leaf index `17` has been spent. etc.
* The hash of the serialized UTXO set after applying the transactions in a block shall match the header's UTXO leafset hash.
---

#### Signatures

* Each kernel shall have a valid signature of the `signature_message` for the kernel's `commitment`.
  * See [kernels.md](./kernels.md) for `signature_message` serialization format
* Each input shall have a valid signature of the message "MWEB" for the input's `commitment`.
* Each output shall have a valid signature of the `output_message` for the output's `sender_pubkey`.
  * `output_message` is serialized as `(features | receiver_pubkey |pub_nonce | encrypted_data)`
* Each signed owner message shall be a valid signature of the hash of a kernel in the block.
---

#### Bulletproofs

* Each output shall be coupled with a bulletproof that proves the commitment is to a value in the range `[0, 2^64)`.
* Each bulletproof shall commit to the `output_message` using its `extra_commit` functionality.
---

#### Kernel Sums

* The sum of all output commitments in the UTXO set at a given block height shall equal the sum of all kernel commitments plus the `total_kernel_offset*G` and the `expected_supply*H` of the block.
  * `sum(utxo.commitments) = sum(kernel.commitments) + (block.total_kernel_offset*G) + (expected_supply*H)`
---

#### Owner Sums

* The sum of all receiver pubkeys for the inputs in a block, plus the sum of all owner pubkeys in the block, plus the `total_owner_offset*G` of the block shall equal the sum of all output sender pubkeys in the block.
  * `sum(input.receiver_pubkey) + sum(owner_pubkey) + (block.total_owner_offset*G) = sum(output.sender_pubkey)`
---

#### Pegging-Out

* Kernels may include an optional pegout, containing the `amount` & a `scriptPubKey` (serialized `CScript`)
  * The `scriptPubKey` shall be a valid `CScript` between 4 and 42 bytes, inclusive.
  * Total MWEB supply shall be reduced by the exact `amount` (in addition to any supply reduction from the `fee`)
  * Miners shall include an output in the block's `HogEx` transaction with the exact `amount` and `scriptPubKey`
---

#### Pegging-In

* Kernels may include an optional pegin `amount`
  * Total MWEB supply shall be increase by the exact `amount`
  * The peg-in kernel must have a corresponding pegin output on the canonical LTC side.
---

#### Block Weight

* Outputs shall be counted as having a weight of 18.
* Kernels shall be counted as having a weight of either 2 (without stealth excess) or 3 (with stealth excess).
  * The `scriptPubKey` shall be a valid `CScript` between 4 and 42 bytes, inclusive.
  * Kernel `extra_data` shall not exceed 100 bytes.
* Extension blocks shall be capped at a maximum total weight of 21,000.
* Inputs shall not contribute toward the block weight.