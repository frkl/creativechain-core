// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "crypto/scrypt.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
#include "sph_keccak.h"
#include "streams.h"

#include "consensus/consensus.h"

uint256 SerializeKeccakHash(const CBlockHeader& obj)
{
    CDataStream ss(SER_GETHASH, PROTOCOL_VERSION);
    ss << obj;

    sph_keccak256_context ctx_keccak;
    uint256 hash;

    sph_keccak256_init(&ctx_keccak);
    sph_keccak256(&ctx_keccak, (void*)&*ss.begin(), ss.size());
    sph_keccak256_close(&ctx_keccak, static_cast<void*>(&hash));

    return hash;
}

uint256 CBlockHeader::GetHash() const
{
    if (nTime < KECCAK_TIME)
        return SerializeHash(*this);
    return SerializeKeccakHash(*this);
}

uint256 CBlockHeader::GetPoWHash() const
{
    uint256 thash;
    if (nTime < KECCAK_TIME) {
        scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    } else {
        thash = SerializeKeccakHash(*this);
    }
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i].ToString() << "\n";
    }
    return s.str();
}

int64_t GetBlockWeight(const CBlock& block)
{
    // This implements the weight = (stripped_size * 4) + witness_size formula,
    // using only serialization with and without witness data. As witness_size
    // is equal to total_size - stripped_size, this formula is identical to:
    // weight = (stripped_size * 3) + total_size.
    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1) + ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
}
