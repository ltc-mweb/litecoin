# Stealth Addresses

Transacting via the MWEB happens via stealth addresses. Stealth addresses are supported using the *Dual-Key Stealth Address Protocol (DKSAP)*.
These addresses consist of 2 secp256k1 public keys (**A<sub>i</sub>**, **B<sub>i</sub>**) where **A<sub>i</sub>** is the scan pubkey used for identifying outputs, and **B<sub>i</sub>** is the spend pubkey.

### Notation
* **G** = Curve generator point
* Bold uppercase letter (**A**, **A<sub>i</sub>**, etc.) represents a secp256k1 public key
* Bold lowercase letter (**a**, **a<sub>i</sub>**, etc.) represents a secp256k1 secret key (scalar)
* Italic lowercase letter (*i*) represents a simple integer
* `HASH(x | y | z)` represents the `BLAKE3` hash of the conjoined serializations of `x`, `y`, and `z`

### Address Generation

Unique stealth addresses should be generated deterministically from a wallet's seed using the following formula (using **G** = generator point):

1. Generate master scan keypair (**a**, **A**) using HD keychain path `m/1/0/100'`
2. Generate master spend keypair (**b**, **B**) using HD keychain path `m/1/0/101'`
3. Choose the lowest unused address index *i*
4. Calculate one-time spend keypair (**b<sub>i</sub>**, **B<sub>i</sub>**) as:<br/>
    **b<sub>i</sub>** = **b** + `HASH(`'A' | *i* | **a**`)`<br/>
    **B<sub>i</sub>** = **b<sub>i</sub>\*G** where **G** refers to the curve generator point
5. Calculate one-time scan keypair (**a<sub>i</sub>**, **A<sub>i</sub>**) as:<br/>
    **a<sub>i</sub>** = **a\*b<sub>i</sub>**<br/>
    **A<sub>i</sub>** = **a<sub>i</sub>\*G** where **G** refers to the curve generator point

### Output Identification

TODO: Document