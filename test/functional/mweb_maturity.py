#!/usr/bin/env python3
# Copyright (c) 2021 The Litecoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""MWEB peg-in maturity test"""

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal

class MWEBMaturityTest(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        self.log.info("Create some blocks")
        self.nodes[0].generate(101)

        self.log.info("Pegin some coins - activate MWEB")
        addr0 = self.nodes[0].getnewaddress(address_type='mweb')
        self.nodes[0].sendtoaddress(addr0, 10)
        self.nodes[0].generate(700)

        self.log.info("Pegin more coins")
        addr1 = self.nodes[0].getnewaddress(address_type='mweb')
        self.nodes[0].sendtoaddress(addr1, 50)
        self.nodes[0].generate(1)

        self.log.info("Check UTXOs")
        utxos = self.nodes[0].listunspent(addresses=[addr1])
        assert len(utxos) == 0

        self.log.info("Create some blocks")
        self.nodes[0].generate(20)

        self.log.info("Check UTXO has matured")
        utxos = self.nodes[0].listunspent(addresses=[addr1])
        assert len(utxos) == 1
        assert utxos[0]['amount'] == 50 and utxos[0]['spendable']

if __name__ == '__main__':
    MWEBMaturityTest().main()
