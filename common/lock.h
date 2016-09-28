#ifndef _LOCK_H__
#define _LOCK_H__

#include <pthread.h>

class RWLockable
{
    public:
        RWLockable() { pthread_rwlock_init( & m_lock, NULL ); }
        ~RWLockable() { pthread_rwlock_destroy( & m_lock ); }

        void RLock() const { pthread_rwlock_rdlock( & m_lock ); }
        void RUnlock() const { pthread_rwlock_unlock( & m_lock ); }

        void WLock() const { pthread_rwlock_wrlock( & m_lock ); }
        void WUnlock() const { pthread_rwlock_unlock( & m_lock ); }

    private:
        mutable pthread_rwlock_t m_lock;

    private:
        // disable
        RWLockable(const RWLockable &);
        const RWLockable & operator = (const RWLockable &);
};

class ScopeRLock
{
    public:
        explicit ScopeRLock( RWLockable & lock ) : m_lock( lock ) {
            m_lock.RLock();
        }

        ~ScopeRLock() {
            m_lock.RUnlock();
        }

    private:
        RWLockable & m_lock;

    private:
        // disable
        ScopeRLock();
        ScopeRLock(const ScopeRLock &);
        const ScopeRLock & operator = (const ScopeRLock &);
};

class ScopeWLock
{
    public:
        explicit ScopeWLock( RWLockable & lock ) : m_lock( lock ) {
            m_lock.WLock();
        }

        ~ScopeWLock() {
            m_lock.WUnlock();
        }

    private:
        RWLockable & m_lock;

    private:
        // disable
        ScopeWLock();
        ScopeWLock(const ScopeWLock &);
        const ScopeWLock & operator = (const ScopeWLock &);
};

#endif
