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

unsigned int GetEpochSeconds() {
    time_t seconds;
    seconds = time(NULL);
    return (unsigned int) seconds;
}

bool IsChangePowTime() {
    unsigned int currtime = GetEpochSeconds();
    return currtime >= CHANGE_POW_TIME;
}

int64_t DifficultyAdjustmentInterval(const Consensus::Params& params) {
    return IsChangePowTime() ? params.DifficultyAdjustmentIntervalV2() : params.DifficultyAdjustmentInterval();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{

    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    int64_t difficultyAdjustmentInterval = DifficultyAdjustmentInterval(params);

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

    // Go back by what we want to be 14 days worth of blocks
    // creativecoin: This fixes an issue where a 51% attack can change difficulty at will.
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

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    if (IsChangePowTime()) {
        //Adjustment diff for Keccak Algorithm
        int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
        arith_uint256 bnPowLimit = UintToArith256(params.nKeccakPowLimit);

        if (nActualTimespan < params.nDigiShieldPowTargetTimespan/4)
            nActualTimespan = params.nDigiShieldPowTargetTimespan/4;
        if (nActualTimespan > params.nDigiShieldPowTargetTimespan*4)
            nActualTimespan = params.nDigiShieldPowTargetTimespan*4;

        arith_uint256 bnNew;

        //Restarting the difficulty with the new PoW as if it were the genesis block
        if (pindexLast->nTime >= CHANGE_POW_TIME) {
            bnNew.SetCompact(pindexLast->nBits);
        } else {
            bnNew.SetCompact(bnPowLimit.GetCompact());
        }

        arith_uint256 bnOld = bnNew;

        bnNew *= nActualTimespan;
        bnNew /= nActualTimespan;

        if (bnNew > bnPowLimit)
            bnNew = bnPowLimit;

        LogPrintf("%s: Calculating new diff: old = %x, new = %x", "New PoW", bnOld.GetCompact(), bnNew.GetCompact());
        return bnNew.GetCompact();
    } else {
        // Retarget
        arith_uint256 bnNew;
        arith_uint256 bnOld;
        bnNew.SetCompact(pindexLast->nBits);
        bnOld = bnNew;
        const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

        // Limit adjustment step
        int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
        if (nActualTimespan < params.nPowTargetTimespan/4)
            nActualTimespan = params.nPowTargetTimespan/4;
        if (nActualTimespan > params.nPowTargetTimespan*4)
            nActualTimespan = params.nPowTargetTimespan*4;


        // creativecoin: intermediate uint256 can overflow by 1 bit
        bool fShift = bnNew.bits() > 235;
        if (fShift)
            bnNew >>= 1;

        bnNew *= nActualTimespan;
        bnNew /= params.nPowTargetTimespan;
        if (fShift)
            bnNew <<= 1;

        if (bnNew > bnPowLimit)
            bnNew = bnPowLimit;

        LogPrintf("%s: Calculating new diff: old = %x, new = %x", "SCrypt", bnOld.GetCompact(), bnNew.GetCompact());
        return bnNew.GetCompact();
    }
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, uint32_t nTime, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);
    uint256 powLimit = nTime >= CHANGE_POW_TIME ? params.nKeccakPowLimit : params.powLimit;

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(powLimit)) {
        return error("fNegative: %d, bnTarget == 0: %d, fOverflow: %d, bnTarget > UintToArith256(powLimit): %d", fNegative, bnTarget == 0, fOverflow, bnTarget > UintToArith256(powLimit));
    }

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("UintToArith256(hash) > bnTarget: %d", UintToArith256(hash) > bnTarget);

    return true;
}
