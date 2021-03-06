//===- MipsTargetMachine.h - Define TargetMachine for Mips ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mips specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H
#define LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H

#include "MCTargetDesc/MipsABIInfo.h"
#include "MipsSubtarget.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/CHERICap.h"
#include <memory>

namespace llvm {

class MipsTargetMachine : public LLVMTargetMachine {
  bool isLittle;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  // Selected ABI
  MipsABIInfo ABI;
  MipsSubtarget *Subtarget;
  MipsSubtarget DefaultSubtarget;
  MipsSubtarget NoMips16Subtarget;
  MipsSubtarget Mips16Subtarget;

  mutable StringMap<std::unique_ptr<MipsSubtarget>> SubtargetMap;

public:
  MipsTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT, bool isLittle);
  ~MipsTargetMachine() override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) override;

  const MipsSubtarget *getSubtargetImpl() const {
    if (Subtarget)
      return Subtarget;
    return &DefaultSubtarget;
  }

  const MipsSubtarget *getSubtargetImpl(const Function &F) const override;

  /// \brief Reset the subtarget for the Mips target.
  void resetSubtarget(MachineFunction *MF);

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  bool isLittleEndian() const { return isLittle; }
  const MipsABIInfo &getABI() const { return ABI; }

  bool isMachineVerifierClean() const override {
    return false;
  }
};

/// Mips32/64 big endian target machine.
///
class MipsebTargetMachine : public MipsTargetMachine {
  virtual void anchor();

public:
  MipsebTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                      CodeGenOpt::Level OL, bool JIT);
};

/// Mips32/64 little endian target machine.
///
class MipselTargetMachine : public MipsTargetMachine {
  virtual void anchor();

public:
  MipselTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                      CodeGenOpt::Level OL, bool JIT);
};

/// MipsCheriTargetMachine - MIPS with CHERI capability extensions.
class MipsCheriTargetMachine : public MipsebTargetMachine {
  void anchor() override;
public:
  MipsCheriTargetMachine(const Target &T, const Triple &TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                        CodeGenOpt::Level OL, bool JIT);
  bool isCompatibleDataLayout(const DataLayout &Candidate) const override {
    if (!isCapabilitySizeCompatible(Candidate))
      return false;
    if (TargetMachine::isCompatibleDataLayout(Candidate))
      return true;
    DataLayout MutableCandidate(Candidate);
    MutableCandidate.setAllocaAS(createDataLayout().getAllocaAddrSpace());
    return TargetMachine::isCompatibleDataLayout(MutableCandidate);
  }

private:
  bool isCapabilitySizeCompatible(const DataLayout &Candidate) const {
    if (!getSubtargetImpl()->isCheri())
      return true;
    switch (Candidate.getPointerSizeInBits(200)) {
    case 64:
      return getSubtargetImpl()->isCheri64();
    case 128:
      return getSubtargetImpl()->isCheri128();
    case 256:
      return getSubtargetImpl()->isCheri256();
    default:
      llvm_unreachable("DataLayout does not have a valid size for AS200");
    }
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H
