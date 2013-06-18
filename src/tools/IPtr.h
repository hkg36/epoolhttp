#include "locks.h"
#include <assert.h>
#ifndef IPTR_H_
#define IPTR_H_
template<class TYPE,class CounterType=InterlockCounter>
class IPtrBase
{
private:
	CounterType m_ref;
public:
	const CounterType& GetRefCount() const{return m_ref;}
	IPtrBase():m_ref(0){};
	long AddRef()
	{
		return ++m_ref;
	}
	long Release()
	{
		long res=--m_ref;
		if(res==0)
		{
			delete static_cast<TYPE*>(this);
		}
		return res;
	}
};
template<class TYPE>
struct IPtrCast
{
	template<class OtherType>
	const TYPE* Cast(const OtherType* far_p) const
	{
		return dynamic_cast<const TYPE*>(far_p);
	}
};
template<class TYPE,class CastPtr=IPtrCast<TYPE> >
class CIPtr
{
private:
	TYPE* point;
public:
	class _NoAddRefReleaseOnCIPtr : public TYPE
	{
	private:
		long AddRef()
		{
			assert(false);
			//do not call this
			return 0;
		}
		long Release()
		{
			assert(false);
			//do not call this
			return 0;
		}
	};
	const TYPE* GetPoint()const
	{return point;}
	CIPtr(const TYPE *p)
	{
		point=const_cast<TYPE*>(p);
		if(point!=NULL)
			point->AddRef();
	}
	~CIPtr()
	{
		if(point!=NULL)
			point->Release();
	}
	CIPtr():point(NULL)
	{
	}
	CIPtr(const CIPtr<TYPE,CastPtr> &cip)
	{
		point=cip.point;
		if(point!=NULL)
			point->AddRef();
	}
	/*CIPtr(CIPtr<TYPE,CastPtr> &&cip)
	{
		point=cip.point;
		cip.point=NULL;
	}*/
	template<class OtherType,class OtherCastPtr>
	CIPtr(const CIPtr<OtherType,OtherCastPtr> &cip):point(NULL)
	{
		TypeCast(cip.GetPoint());
	}
	template<class OtherType>
	CIPtr(const OtherType *p):point(NULL)
	{
		TypeCast(p);
	}
protected:
	template<class OtherType>
	TYPE* TypeCast(const OtherType* far_p)
	{
		CastPtr ptrcast;
		const TYPE* temp=ptrcast.Cast(far_p);
		return (*this)=temp;
	}
public:
	template<class OtherType>
	_NoAddRefReleaseOnCIPtr* operator=(const OtherType *farp)
	{
		return (_NoAddRefReleaseOnCIPtr*)TypeCast(farp);
	}
	template<class OtherType,class OtherCastPtr>
	_NoAddRefReleaseOnCIPtr* operator=(const CIPtr<OtherType,OtherCastPtr> &cip)
	{
		return (_NoAddRefReleaseOnCIPtr*)TypeCast(cip.GetPoint());
	}
	_NoAddRefReleaseOnCIPtr* operator=(const CIPtr<TYPE,CastPtr> &cip)
	{
		if(point!=cip.point)
		{
			if(point!=NULL)
				point->Release();
			point=const_cast<TYPE*>(cip.GetPoint());
			if(point!=NULL)
				point->AddRef();
		}
		return (_NoAddRefReleaseOnCIPtr*)(TYPE*)point;
	}
	_NoAddRefReleaseOnCIPtr* operator=(const TYPE *farp)
	{
		if(point!=farp)
		{
			if(point!=NULL)
				point->Release();
			point=const_cast<TYPE*>(farp);
			if(point!=NULL)
				point->AddRef();
		}
		return (_NoAddRefReleaseOnCIPtr*)(TYPE*)point;
	}
	operator TYPE*() const
	{
		return point;
	}
	_NoAddRefReleaseOnCIPtr* operator->() const
	{
		assert(point!=NULL);
		return (_NoAddRefReleaseOnCIPtr*)(TYPE*)point;
	}
};
#endif //IPTR_H_
