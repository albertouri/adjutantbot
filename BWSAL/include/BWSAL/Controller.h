#pragma once
#include <set>
#include <string>
namespace BWSAL
{
  template < class _Tp, class _Val > 
  class Controller
  {
  public:
    virtual void onOffer( std::set< _Tp > objects ) = 0;
    virtual void onRevoke( _Tp, _Val bid ) = 0;
    virtual std::string getName() const = 0;
  };
}