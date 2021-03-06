// @file key.h class(es) representing individual keys in a btree

/**
*    Copyright (C) 2011 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
 
#include "jsobj.h"

namespace mongo { 

    /** Key class for precomputing a small format index key that is denser than a traditional BSONObj. 

        KeyBson is a legacy wrapper implementation for old BSONObj style keys for v:0 indexes.

        KeyV1 is the new implementation.
    */
    class KeyBson { 
    public:
        KeyBson() { }
        explicit KeyBson(const char *keyData) : _o(keyData) { }
        explicit KeyBson(const BSONObj& obj) : _o(obj) { }
        int woCompare(const KeyBson& r, const Ordering &o) const { return _o.woCompare(r._o, o); }
        bool woEqual(const KeyBson& r) const { return _o.woEqual(r._o); }
        BSONObj toBson() const { return _o; }
        string toString() const { return _o.toString(); }
        int dataSize() const { return _o.objsize(); }
        const char * data() const { return _o.objdata(); }
        BSONElement _firstElement() const { return _o.firstElement(); }
        bool isCompactFormat() const { return false; }
    private:
        BSONObj _o;
    };

    class KeyV1 { 
    public:
        KeyV1() { _keyData = 0; }
        ~KeyV1() { DEV _keyData = (const unsigned char *) 1; }

        /** @param keyData can be a buffer containing data in either BSON format, OR in KeyV1 format. 
                   when BSON, we are just a wrapper
        */
        explicit KeyV1(const char *keyData) : _keyData((unsigned char *) keyData) { }
        int woCompare(const KeyV1& r, const Ordering &o) const;
        bool woEqual(const KeyV1& r) const;
        BSONObj toBson() const;
        string toString() const { return toBson().toString(); }

        /** get the key data we want to store in the btree bucket */
        const char * data() const { return (const char *) _keyData; }

        /** @return size of data() */
        int dataSize() const;

        /** only used by geo, which always has bson keys */
        BSONElement _firstElement() const { return bson().firstElement(); }
        bool isCompactFormat() const { return *_keyData != IsBSON; }
    protected:
        enum { IsBSON = 0xff };
        const unsigned char *_keyData;
        BSONObj bson() const {
            dassert( !isCompactFormat() );
            return BSONObj((const char *) _keyData+1);
        }
    private:
        int compareHybrid(const KeyV1& right, const Ordering& order) const;
    };

    class KeyV1Owned : public KeyV1 { 
    public:
        /** @obj a BSON object to be translated to KeyV1 format.  If the object isn't 
                 representable in KeyV1 format (which happens, intentionally, at times)
                 it will stay as bson herein.
        */
        KeyV1Owned(const BSONObj& obj);
        ~KeyV1Owned() { free((void*) _toFree); }
    private:
        KeyV1Owned(const KeyV1Owned&); //not copyable
        const char *_toFree;
        void traditional(BufBuilder& b, const BSONObj& obj); // store as traditional bson not as compact format
    };

};
