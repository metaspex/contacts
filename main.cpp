//
// Copyright Metaspex - 2023
// mailto:admin@metaspex.com
//

#include "hx2a/element.hpp"
#include "hx2a/root.hpp"
#include "hx2a/slot.hpp"
#include "hx2a/own.hpp"
#include "hx2a/db/connector.hpp"

// We reuse the address type from the Foundation Ontology. Consult the documentation.
#include "hx2a/components/address.hpp"

#include "hx2a/service.hpp"
#include "hx2a/paginated_services.hpp"
// For convenience, we reuse the Foundation Ontology's ready-made query_id type which accepts a
// JSON query payload as:
// {"id": "---the doc_id---"}
#include "hx2a/payloads/query_id.hpp"
// For convenience, we reuse the Foundation Ontology's ready-made reply_id type which serializes a
// JSON response as:
// {"id": "---the doc_id---"}
#include "hx2a/payloads/reply_id.hpp"

/*
  The code below is a Metaspex specification. It is also code. It is a specification in the sense that it essentially describes
  the "what" and not the "how". It is code in the sense that it is not a set of diagrams, it is a piece of formal text which is 
  validated and compiled using an off the shelf compiler.

  The specification below describes the "what" in the sense that it stands very high in terms of abstraction and is very precise,
  even more so than classical relational database schemas. We remove toil to reach multiple orders of magnitude of productivity
  boost.

  Below, you'll find:

  - No SQL and more generally no query language of any kind. The specification is entirely object-oriented, no cross-cut of the
    data model is made. Types in the ontology are completely reusable. They constitute "knowledge acquisition".
  - No low-level database-related code is present, no trace of Couchbase, MongoDB or CouchDB drivers, although all of these products
    are supported to make the information persistent.
    The mapping of logical database names below to physical databases is described in the configuration file. The object file produced 
    from the specification below is independent from physical databases and can be deployed on multiple databases.
  - No explicit commit or rollback is present.
  - No network-related code is present. Yet, the code below produces HTTP REST JSON in/JSON out services, optionally supporting
    TLS or SSL.
  - No reference is made to Apache or Nginx, yet the code below is compiled as Apache or Nginx plug-ins.
  - No password is specified, the binary produced is independent from them. They are exclusively in the configuration file(s).
  - No data presentation is specified, no JSON, no BSON (MongoDB). This is automatic.
  - No error-prone serialization/deserialization for the protocols above are specified. They are automatically generated.
  - No explicit objects memory deletion is present, objects collection is done automatically in a way faster than garbage-collected
    programming languages do it, and even faster than generalized reference counting techniques. Metaspex allows cycles.
  - Referential integrity is verified at compile time and dynamically also. It is also actively maintained without any code.

  It can be helpful to have Metaspex's reference guide open to understand each of the constructs used.
 */

using namespace hx2a;

namespace contacts {

  // Contacts ontology.

  class contact;
  using contact_p = ptr<contact>; // Definitions to shorten the notation.
  using contact_r = rfr<contact>;
  
  class contact: public root<> // A contact is at the top of a document.
  {
    HX2A_ROOT(
	      contact, // Name of the type. Repeats the type above.
	      "cont", // Tag which will be used to uniquely identify the type in the database.
	      1, // This is the first version of the contact type. Floating point number to be increased at every layout change.
	      root, // Metaspex base type. Repeats the base type above.
	      ((_first_name, "fn"), // JSON tag of the slot, as it will appear in the database.
	       (_middle_name, "mn"),
	       (_last_name, "ln"),
	       (_address, "addr"))); // Members in meta.
  public:
    
    // The ctor takes an address_r to require a non-null address.
    contact(string fn, string mn, string ln, const address_r& addr):
      _first_name(*this, std::move(fn)),
      _middle_name(*this, std::move(mn)),
      _last_name(*this, std::move(ln)),
      _address(*this, addr) // A own relationship takes an address_p.
    {
    }

    // Accessors, for convenience.
    const string& get_first_name() const { return _first_name; }
    const string& get_middle_name() const { return _middle_name; }
    const string& get_last_name() const { return _last_name; }
    
    // We know the address is non null, we can convert to an address_r.
    address_r get_address() const { return *_address; }
    
    // A null address means that no address update must take place.
    void update(string fn, string mn, string ln, const address_p& addr){
      _first_name = std::move(fn);
      _middle_name = std::move(mn);
      _last_name = std::move(ln);
      
      if (addr != nullptr){
	_address = addr;
      }
    }
    
  private:
    
    slot<
      string // Type of the slot.
    > _first_name;
    slot<string> _middle_name;
    slot<string> _last_name;
    // The address of the contact, ready-made from Metaspex's Foundation Ontology.
    own<address> _address;
  };
  
  // Specification of service payloads.
  
  class contact_info_payload;
  using contact_info_payload_p = ptr<contact_info_payload>;
  using contact_info_payload_r = rfr<contact_info_payload>;
  
  class contact_info_payload: public element<>
  {
    HX2A_ELEMENT(contact_info_payload, "contact_info_pld", element,
		 ((_first_name, "first_name"),
		  (_middle_name, "middle_name"),
		  (_last_name, "last_name"),
		  (_address, "address")));
  public:
    
    contact_info_payload(const contact_r& cont):
      _first_name(*this, cont->get_first_name()),
      _middle_name(*this, cont->get_middle_name()),
      _last_name(*this, cont->get_last_name()),
      _address(*this, cont->get_address()->copy()) // The address is owned by the contact, we must copy it.
    {
    }
    
    // The tags chosen are different from the ones used for contact persistence. Here they are readable.
    // They could be shortened for network frugality.
    slot<string> _first_name;
    slot<string> _middle_name;
    slot<string> _last_name;
    // The payload will carry the contact's address as a sub JSON object.
    own<address> _address;
  };
  
  // It's the same contents. Alias for elegance.
  using contact_create_payload = contact_info_payload;
  
  // For update we need to add the document identifier of the contact. We reuse the contact_info_payload
  // already defined.
  class contact_update_payload: public contact_info_payload
  {
    HX2A_ELEMENT(contact_update_payload, "contact_update_pld", contact_info_payload,
		 ((_id, "id")));
  public:
    
    slot<doc_id> _id;
  };
  
  // Specification of the services.
  // No explicit destruction of any object below, Metaspex takes care of this automatically as a byproduct of referential
  // integrity.
  // No explicit rollback anywhere, rollback is done automatically in case of exception.
  
  // Creation of a contact.
  auto _contact_create = service<"contact_create"> // The name of the service as it'll appear at the end of the URI accepted by the server.
    ([](const rfr<contact_create_payload>& ccp){
      db::connector c{"hx2a"}; // Connector to the database described in the configuration file under the logical name "hx2a".
      
      // A contact constructor takes a non-null address, we must make sure the client did not forget to send one.
      address_p opt_ccpaddr = ccp->_address;
      
      if (opt_ccpaddr == nullptr){
	// Throwing an exception will stop the service and send back a graceful error message to the client, with specifics about
	// the issue. See the reference manual for address_missing exception.
	throw address_missing();
      }

      // Non null address.
      address_r ccpaddr = *opt_ccpaddr;
      
      // Creation of the contact. As we have a connector it'll persist in the "hx2a" database.
      // We could write the two lines below as a single one. Using two for readability.
      // We copy the address because the payload's address is owned by the payload. Alternatively we could cut the address
      // from the payload and use it as-is.
      contact_r cont = make<contact>(c, ccp->_first_name, ccp->_middle_name, ccp->_last_name, ccpaddr->copy());
      
      // Returning the document identifier of the newly-created contact to the client.
      return make<reply_id>(cont->get_id());
      
      // No commit, it's done automatically by Metaspex at the end of a successful service call (without exception thrown).
    });
  
  // Retrieval of a contact from its document identifier.
  auto _contact_retrieve = service<"contact_retrieve">
    ([](const rfr<query_id>& q){
      db::connector c{"hx2a"};
      
      // Retrieving the contact. It is a ptr and not a rfr because the document might not exist and get will return null.
      contact_r cont = root_get<contact>(c, q->get_id()).or_throw<document_does_not_exist>();

      // At this point we could return the contact as-is. As we have a more substantial application with paginated
      // services below, we create a proper payload which reflects the contents of the contact.
      // Returning the payload giving information about the contact.
      return make<contact_info_payload>(cont);
    });
  
  // Update of a contact. All data at once to avoid multiplying the number of services (one for updating each piece of data).
  auto _contact_update = service<"contact_update">
    ([](const rfr<contact_update_payload>& q){
      db::connector c{"hx2a"};
      
      // Retrieving the contact. It is a ptr and not a rfr because the document might not exist and get will return null.
      contact_r cont = root_get<contact>(c, q->_id).or_throw<document_does_not_exist>();
      
      // If the address is missing from the payload, no address update will happen.

      address_p address;

      if (q->_address){
	address = q->_address->copy(); // The address is owned by the payload, we must copy it.
      }
      
      cont->update(
		   q->_first_name,
		   q->_middle_name,
		   q->_last_name,
		   address
		   );
      
      // Metaspex will figure out if any update happened. Assigning a slot to its current value does not do anything, and does not
      // earmark the document it belongs to as modified. So in case the update is not changing any piece of data, the contact
      // document will not be written in the database.
    });

  // Deletion of a contact from its document identifier.
  auto _contact_delete = service<"contact_delete">
    ([](const rfr<query_id>& q){
      db::connector c{"hx2a"};
      
      // Retrieving the contact. It is a ptr and not a rfr because the document might not exist and get will return null.
      contact_r cont = root_get<contact>(c, q->get_id()).or_throw<document_does_not_exist>();
      
      // This marks the document for removal, except if a rollback happens before the end of the service. A rollback is automatically
      // triggered in case of exception. As we return right after, the document will be removed.
      cont->unpublish();
    });
  
  // Pagination services on contact last names, first name, middle name and city name.
  // This creates three distinct services which allow to navigate among contacts.
  
  // Functor to calculate contact info payload from a contact.
  struct compute_contact_info_payload
  {
    // Returns payload to be returned by contact.
    contact_info_payload_r operator()(const contact_r& source) const {
      // We simply use the ctor we defined.
      return make<contact_info_payload>(source);
    }
  };

  // Definition of the 3 paginated services, open, next and prev.
  paginated_services<
    "contact_list", // Prefix for the 3 paginated services names.
    contact, // Type in the ontology the paginated services will scan.
    compute_contact_info_payload // Functor to calculate the contact info payload from the contact.
    >
  _contact_list(
		config::get_id("hx2a"), // UUID of the database on which to iterate.
		"contacts_per_last_name" // Logical name of the index to be used. Consult the configuration file for its physical description.
		); // Singleton.
  
} // End namespace contacts.
