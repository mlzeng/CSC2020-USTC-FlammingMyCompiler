#ifndef SYSYC_HIGHBLOCK_H
#define SYSYC_HIGHBLOCK_H

#include "BaseBlock.h"
#include "Value.h"

class IfBlock : public BaseBlock {
public:
  // this will auto add this to function's baseblock list
  // don't use this in nested structure
  static IfBlock *create(Module *m, const std::string &name, Function *func) {
    return new IfBlock(m, name, func);
  }
  static IfBlock *create(Module *m, const std::string &name) {
    return new IfBlock(m, name);
  }

  void addCondBB(BaseBlock *bb) {
    cond_.push_back(bb);
    bb->setBaseFather(this);
  }
  void addIfBodyBB(BaseBlock *bb) {
    if_body_.push_back(bb);
    bb->setBaseFather(this);
  }
  void addElseBodyBB(BaseBlock *bb) {
    else_body_.push_back(bb);
    bb->setBaseFather(this);
  }

  std::list<BaseBlock *> &getCondBBs() { return cond_; }
  std::list<BaseBlock *> &getIfBodyBBs() { return if_body_; }
  std::list<BaseBlock *> &getElseBodyBBs() { return else_body_; }

  void removeCondBB(BaseBlock *bb) { cond_.remove(bb); }
  void removeIfBodyBB(BaseBlock *bb) { if_body_.remove(bb); }
  void removeElseBodyBB(BaseBlock *bb) { else_body_.remove(bb); }
  void removeBB(BaseBlock *bb) {
    removeCondBB(bb);
    removeIfBodyBB(bb);
    removeElseBodyBB(bb);
  }

  void insertCondBB(std::list<BaseBlock *>::iterator it, BaseBlock *bb) {
    cond_.insert(it, bb);
    bb->setBaseFather(this);
  }
  void insertIfBodyBB(std::list<BaseBlock *>::iterator it, BaseBlock *bb) {
    if_body_.insert(it, bb);
    bb->setBaseFather(this);
  }
  void insertElseBodyBB(std::list<BaseBlock *>::iterator it, BaseBlock *bb) {
    else_body_.insert(it, bb);
    bb->setBaseFather(this);
  }

  void clearCondBBs() { cond_.clear(); }
  // TODO
  void print() override {}

private:
  explicit IfBlock(Module *m, const std::string &name, Function *func)
      : BaseBlock(BaseBlock::If, m, name, func) {}
  explicit IfBlock(Module *m, const std::string &name)
      : BaseBlock(BaseBlock::If, m, name) {}

  std::list<BaseBlock *> cond_;
  std::list<BaseBlock *> if_body_;
  std::list<BaseBlock *> else_body_;
};

class WhileBlock : public BaseBlock {
public:
  // this will auto add this to function's baseblock list
  // don't use this in nested structure
  static WhileBlock *create(Module *m, const std::string &name,
                            Function *func) {
    return new WhileBlock(m, name, func);
  }
  static WhileBlock *create(Module *m, const std::string &name) {
    return new WhileBlock(m, name);
  }

  void addCondBB(BaseBlock *bb) {
    cond_.push_back(bb);
    bb->setBaseFather(this);
  }
  void addBodyBB(BaseBlock *bb) {
    body_.push_back(bb);
    bb->setBaseFather(this);
  }

  std::list<BaseBlock *> &getCondBBs() { return cond_; }
  std::list<BaseBlock *> &getBodyBBs() { return body_; }

  void removeCondBB(BaseBlock *bb) { cond_.remove(bb); }
  void removeWhileBodyBB(BaseBlock *bb) { body_.remove(bb); }
  void removeBB(BaseBlock *bb) {
    removeCondBB(bb);
    removeWhileBodyBB(bb);
  }
  // TODO
  void insertCondBB(std::list<BaseBlock *>::iterator it, BaseBlock *bb) {
    cond_.insert(it, bb);
    bb->setBaseFather(this);
  }
  void insertWhileBodyBB(std::list<BaseBlock *>::iterator it, BaseBlock *bb) {
    body_.insert(it, bb);
    bb->setBaseFather(this);
  }

  void print() override {}

private:
  explicit WhileBlock(Module *m, const std::string &name, Function *func)
      : BaseBlock(BaseBlock::While, m, name, func) {}
  explicit WhileBlock(Module *m, const std::string &name)
      : BaseBlock(BaseBlock::While, m, name) {}

  std::list<BaseBlock *> cond_;
  std::list<BaseBlock *> body_;
};

#endif // SYSYC_HIGHBLOCK_H
