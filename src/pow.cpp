// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chrono>
#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"
#include "consensus/consensus.h"

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{

    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    uint256 powLimit = pindexLast->nTime >= KECCAK_TIME ? params.nKeccakPowLimit : params.powLimit;
    nProofOfWorkLimit = UintToArith256(powLimit).GetCompact();

    int nHeight = pindexLast->nHeight+1;
    int64_t difficultyAdjustmentInterval = nHeight >= params.nDigiShieldHeight ? params.DifficultyAdjustmentIntervalV2() : params.DifficultyAdjustmentInterval();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % difficultyAdjustmentInterval != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % difficultyAdjustmentInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }


    // Creativecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = difficultyAdjustmentInterval-1;
    if ((pindexLast->nHeight+1) != difficultyAdjustmentInterval)
        blockstogoback = difficultyAdjustmentInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);

}

unsigned int GetEpochSeconds() {
    time_t seconds;
    seconds = time(NULL);
    return (unsigned int) seconds;
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    int height = pindexLast->nHeight+1;
    int64_t powTargetTimespan = height >= params.nDigiShieldHeight ? params.nDigiShieldPowTargetTimespan : params.nPowTargetTimespan;
    unsigned int currtime = GetEpochSeconds();
    bool isKeccakTime = currtime >= KECCAK_TIME;

    uint256 powLimit = isKeccakTime ? params.nKeccakPowLimit : params.powLimit;

    if (isKeccakTime) {
        // Limit adjustment step
        int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
        if (nActualTimespan < powTargetTimespan/4)
            nActualTimespan = powTargetTimespan/4;
        if (nActualTimespan > powTargetTimespan*4)
            nActualTimespan = powTargetTimespan*4;

        // Retarget
        const arith_uint256 bnPowLimit = UintToArith256(powLimit);
        arith_uint256 bnNew;
        bnNew.SetCompact(pindexLast->nBits);
        bnNew *= nActualTimespan;
        bnNew /= powTargetTimespan;

        if (bnNew > bnPowLimit)
            bnNew = bnPowLimit;

        error("Next Diff with DS: %x", bnNew.GetCompact());
        return bnNew.GetCompact();
    } else {
        // Limit adjustment step
        int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
        if (nActualTimespan < powTargetTimespan/4)
            nActualTimespan = powTargetTimespan/4;
        if (nActualTimespan > powTargetTimespan*4)
            nActualTimespan = powTargetTimespan*4;

        // Retarget
        arith_uint256 bnNew;
        arith_uint256 bnOld;
        bnNew.SetCompact(pindexLast->nBits);
        bnOld = bnNew;
        // creativecoin: intermediate uint256 can overflow by 1 bit
        bool fShift = bnNew.bits() > 235;
        if (fShift)
            bnNew >>= 1;
        bnNew *= nActualTimespan;
        bnNew /= powTargetTimespan;
        if (fShift)
            bnNew <<= 1;

        const arith_uint256 bnPowLimit = UintToArith256(powLimit);
        if (bnNew > bnPowLimit)
            bnNew = bnPowLimit;

        error("Next Diff with SC: %x", bnNew.GetCompact());
        return bnNew.GetCompact();
    }


}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, uint32_t nTime, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);
    uint256 powLimit = nTime >= KECCAK_TIME ? params.nKeccakPowLimit : params.powLimit;

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(powLimit)) {
        return error("fNegative: %d, bnTarget == 0: %d, fOverflow: %d, bnTarget > UintToArith256(powLimit): %d", fNegative, bnTarget == 0, fOverflow, bnTarget > UintToArith256(powLimit));
    }

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("UintToArith256(hash) > bnTarget: %d", UintToArith256(hash) > bnTarget);

    return true;
}
