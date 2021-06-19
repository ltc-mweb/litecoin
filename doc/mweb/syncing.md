# MWEB Syncing

### Description

The Mimblewimble protocol allows for a lighter initial block download (IBD), by allowing nodes to skip the downloading of spent TXOs (transaction outputs) while still providing similar security properties as traditional blockchains.

To take advantage of this property, we use the following process when syncing a new LTC node from scratch:

<ol>
<li>Sync headers (unchanged)</li>
<li>
    Sync blocks
    <ul>
        <li>Include kernels - <i>For verification of peg-ins, peg-outs, kernel signatures, and kernel MMR</i></li>
        <li>Include input PMMR indices and output hashes - <i>For verification of output PMMR and utxo bitset</i></li>
    </ul>
</li>
<li>Sync UTXOs (with rangeproofs)</li>
</ol>

### P2P Protocol