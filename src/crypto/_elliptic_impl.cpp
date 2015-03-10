class public_key_impl
{
    public:
        public_key_impl() : _key(nullptr)
        {
            init_lib();
        }

        public_key_impl( const public_key_impl& cpy ) : _key(nullptr)
        {
            init_lib();
            *this = cpy;
        }

        public_key_impl( public_key_impl&& cpy ) : _key(nullptr)
        {
            init_lib();
            *this = cpy;
        }

        ~public_key_impl()
        {
            free_key();
        }

        public_key_impl& operator=( const public_key_impl& pk )
        {
            if (pk._key == nullptr)
            {
                free_key();
            } else if ( _key == nullptr ) {
                _key = dup_key( pk._key );
            } else {
                copy_key( _key, pk._key );
            }
            return *this;
        }

        public_key_impl& operator=( public_key_impl&& pk )
        {
            free_key();
            _key = pk._key;
            pk._key = nullptr;
            return *this;
        }

        pub_data_type* _key;

    private:
        void free_key();
        pub_data_type* dup_key( const pub_data_type* cpy );
        void copy_key( pub_data_type* to, const pub_data_type* from );
};

class private_key_impl
{
    public:
        private_key_impl() : _key(nullptr)
        {
            init_lib();
        }

        private_key_impl( const private_key_impl& cpy ) : _key(nullptr)
        {
            init_lib();
            *this = cpy;
        }

        private_key_impl( private_key_impl&& cpy ) : _key(nullptr)
        {
            init_lib();
            *this = cpy;
        }

        ~private_key_impl()
        {
            free_key();
        }

        private_key_impl& operator=( const private_key_impl& pk )
        {
            if (pk._key == nullptr)
            {
                free_key();
            } else if ( _key == nullptr ) {
                _key = dup_key( pk._key );
            } else {
                copy_key( _key, pk._key );
            }
            return *this;
        }

        private_key_impl& operator=( private_key_impl&& pk )
        {
            free_key();
            _key = pk._key;
            pk._key = nullptr;
            return *this;
        }

        priv_data_type* _key;

    private:
        void free_key();
        priv_data_type* dup_key( const priv_data_type* cpy );
        void copy_key( priv_data_type* to, const priv_data_type* from );
};
