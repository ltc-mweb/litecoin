#pragma once

#include <mw/crypto/PublicKeys.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/TxBody.h>

class OwnerSumValidator
{
public:
    static void Validate(const BlindingFactor& owner_offset, const TxBody& body)
    {
        std::vector<PublicKey> output_pubkeys;
        std::transform(
            body.GetOutputs().cbegin(), body.GetOutputs().cend(), std::back_inserter(output_pubkeys),
            [](const Output& output) { return output.GetSenderPubKey(); }
        );

        std::vector<PublicKey> input_pubkeys;
        std::transform(
            body.GetInputs().cbegin(), body.GetInputs().cend(), std::back_inserter(input_pubkeys),
            [](const Input& input) { return input.GetPubKey(); }
        );

        std::transform(
            body.GetOwnerSigs().cbegin(), body.GetOwnerSigs().cend(), std::back_inserter(input_pubkeys),
            [](const SignedMessage& owner_sig) { return owner_sig.GetPublicKey(); }
        );

        if (!owner_offset.IsZero()) {
            input_pubkeys.push_back(PublicKeys::Calculate(owner_offset.GetBigInt()));
        }

        PublicKey total_input_pubkey, total_output_pubkey;
        if (!input_pubkeys.empty()) {
            total_input_pubkey = PublicKeys::Add(input_pubkeys);
        }
        if (!output_pubkeys.empty()) {
            total_output_pubkey = PublicKeys::Add(output_pubkeys);
        }

        // inputs.pubkeys + owner_sigs.pubkeys + (owner_offset*G) = outputs.pubkeys
        if (total_input_pubkey != total_output_pubkey) {
            ThrowValidation(EConsensusError::OWNER_SUMS);
        }
    }
};