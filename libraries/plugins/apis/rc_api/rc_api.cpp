#include <steem/plugins/rc_api/rc_api_plugin.hpp>
#include <steem/plugins/rc_api/rc_api.hpp>

#include <steem/plugins/rc/rc_objects.hpp>

#include <fc/variant_object.hpp>
#include <fc/reflect/variant.hpp>

namespace steem { namespace plugins { namespace rc {

namespace detail {

class rc_api_impl
{
   public:
      rc_api_impl() : _db( appbase::app().get_plugin< steem::plugins::chain::chain_plugin >().db() ) {}

      DECLARE_API_IMPL
      (
         (get_resource_params)
         (get_resource_pool)
         (find_rc_accounts)
      )

      chain::database& _db;
};

DEFINE_API_IMPL( rc_api_impl, get_resource_params )
{
   get_resource_params_return result;
   fc::mutable_variant_object mvo;
   const rc_resource_param_object& params_obj = _db.get< rc_resource_param_object, by_id >( rc_resource_param_object::id_type() );

   for( size_t i=0; i<STEEM_NUM_RESOURCE_TYPES; i++ )
   {
      mvo( fc::reflector< rc_resource_types >::to_string( i ), params_obj.resource_param_array[i] );
   }

   result.resource_params = mvo;
   return result;
}

DEFINE_API_IMPL( rc_api_impl, get_resource_pool )
{
   get_resource_pool_return result;
   fc::mutable_variant_object mvo;
   const rc_pool_object& pool_obj = _db.get< rc_pool_object, by_id >( rc_pool_object::id_type() );

   for( size_t i=0; i<STEEM_NUM_RESOURCE_TYPES; i++ )
   {
      resource_pool_api_object api_pool;
      api_pool.pool = pool_obj.pool_array[i];
      mvo( fc::reflector< rc_resource_types >::to_string( i ), api_pool );
   }

   result.resource_pool = mvo;
   return result;
}

DEFINE_API_IMPL( rc_api_impl, find_rc_accounts )
{
   find_rc_accounts_return result;

   FC_ASSERT( args.accounts.size() <= RC_API_SINGLE_QUERY_LIMIT );

   for( const account_name_type& a : args.accounts )
   {
      const rc_account_object* rc_account = _db.find< rc_account_object, by_name >( a );

      if( rc_account == nullptr )
         continue;

      rc_account_api_object api_rc_account;
      api_rc_account.account = rc_account->account;
      api_rc_account.rc_manabar = rc_account->rc_manabar;
      api_rc_account.max_rc_creation_adjustment = rc_account->max_rc_creation_adjustment;
      api_rc_account.max_rc = rc_account->max_rc;

      result.rc_accounts.emplace_back( api_rc_account );
   }

   return result;
}

} // detail

rc_api::rc_api(): my( new detail::rc_api_impl() )
{
   JSON_RPC_REGISTER_API( STEEM_RC_API_PLUGIN_NAME );
}

rc_api::~rc_api() {}

DEFINE_READ_APIS( rc_api,
   (get_resource_params)
   (get_resource_pool)
   (find_rc_accounts)
   )

} } } // steem::plugins::rc
