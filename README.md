Warning: you need the [Metaspex](http://metaspex.com) product to compile this application. Please contact us at info@metaspex.com to get access to Metaspex.

This sample Metaspex application leverages documents persistence.
It is a contacts management application similar to what you can find on regular operating systems.
All contacts are stored in a database. You can create, retrieve, update and delete contacts.
7 services are proposed.

To compile:

    make

It will compile the application for use alongside Apache and Nginx, both in debug and optimized
versions.

The `hx2a.conf` file is by default set to perform persistence in [Couchbase](https://www.couchbase.com/), which must be installed before running this sample, if you choose to run it on this database.
The configuration file contains dummy credentials (bucket user/password), they need to be updated to match the credentials you chose when installing Couchbase.
Please consult this reference documentation page to do so (copy/paste the URI in your browser):

    /usr/local/metaspex/doc/reference/hx2a.conf.html

In particular, have a look at the `database` and `index` keywords.

You can also change the database to [MongoDB](https://www.mongodb.com/) or [CouchDB](https://couchdb.apache.org/).
Do not change the logical name `hx2a`, it is used in the application source.
You do not have to change the UUID of the database. Changing it would be required in production.

The creation of the database and the index are made in the `configure.sh` script supplied.
Consult the code of the script and update the credentials and possibly other configuration parameters.
Then run it:

    ./configure.sh

In production the UUID of the database should be obtained through a call to the `hx2a` utility. Consult:

    /usr/local/metaspex/doc/reference/mkdb.html

Go down (`cd`) into the Web server of your choice (`apache` or `nginx`) and the variant (`debug` or `opt`).

To load (or reload) the module in the Web server:

    make server

To test that the module was properly installed (use port 8080 for the default Apache configuration and 8081 for Nginx):

    curl http://localhost:8081/_

It should display the `"Ave"` message, which is something like:

    {"hx2a":"Ave","Nginx version":"1.20.2","hx2a version":"2.0.3","hx2a build date":"Mar 14 2022 09:01:41"}

Testing the various contact handling services yields something like:

#### Contact creation

Omitting the address

    $ curl http://localhost:8081/contact_create -d '{"fn": "John", "ln": "Smith"}'
    {"@exc":{"code":97,"msg":"Address missing in input."}}

With a proper address

    $ curl http://localhost:8081/contact_create -d '{"first_name": "John", "last_name": "Smith", "address": {"fl": "1 Metaspex Way", "pc": "87501", "cty": "Santa Fe", "reg": "NM"}}'
    {"id":"5d38b96402424dc0992a1a58a4f075f2"}

The call is successful. Looking at the corresponding document from Couchbase's Console using `5d38b96402424dc0992a1a58a4f075f2` as the document identifier, we find:

    {
      "@T": {
        "cont": 0
      },
      "@V": 1,
      "@R": 1,
      "@C": 1687709992,
      "@S": 1687709992,
      "@v": {},
      "fn": "John",
      "mn": "",
      "ln": "Smith",
      "addr": {
        "@T": "@A",
        "rcpt": "",
        "fl": "1 Metaspex Way",
        "sl": "",
        "pc": "87501",
        "cty": "Santa Fe",
        "reg": "NM",
        "ctry": 0
      }
    }

The `@C` contains the Unix timestamp of the creation of the contact.

Note that Couchbase's console has a bug which causes the console to be unable to display or search a document. You need to refresh the screen numerous times to be able to look for a document.

We find the JSON tags we specified in the ontology. They are, as specified, different from the ones in the payload. For instance the address tag is `addr` instead of `address`.

#### Contact retrieval

Trying to retrieve a contact that does not exist (the two last digits of the document identifier we just got are replaced by `55`):

    $ curl http://localhost:8081/contact_retrieve -d '{"id": "5d38b96402424dc0992a1a58a4f07555"}'
    {"@exc":{"code":196,"msg":"No document found."}}

Now supplying the right document identifier:

    $ curl http://localhost:8081/contact_retrieve -d '{"id": "5d38b96402424dc0992a1a58a4f075f2"}'
    {"first_name":"John","middle_name":"","last_name":"Smith","address":{"rcpt":"","fl":"1 Metaspex Way","sl":"","pc":"87501","cty":"Santa Fe","reg":"NM","ctry":0}}

Here also, the JSON tags are the ones specified in the `contact_info` type.

#### Contact update

Changing the street number from `1` to `2`:

    $ curl http://localhost:8081/contact_update -d '{"id": "5d38b96402424dc0992a1a58a4f075f2", "address": {"fl": "2 Metaspex Way", "pc": "87501", "cty": "Santa Fe", "reg": "NM"}}'
    {}

We get the empty JSON object, as specified.

Retrieving the updated document using the former service yields:

    $ curl http://localhost:8081/contact_retrieve -d '{"id": "5d38b96402424dc0992a1a58a4f075f2"}'
    {"first_name":"John","middle_name":"","last_name":"Smith","address":{"rcpt":"","fl":"2 Metaspex Way","sl":"","pc":"87501","cty":"Santa Fe","reg":"NM","ctry":0}}

The update happened.

#### Contact deletion

    $ curl http://localhost:8081/contact_delete -d '{"id": "5d38b96402424dc0992a1a58a4f075f2"}'
    {}

Trying to retrieve the document after that yields:

    $ curl http://localhost:8081/contact_retrieve -d '{"id": "5d38b96402424dc0992a1a58a4f075f2"}'
    {"@exc":{"code":196,"msg":"No document found."}}

#### Browsing through contacts (using paginated services)

When no contacts are present, here is what the paginated services open service yields:

    $ curl http://localhost:8081/contact_list -d '{}'
    {}

None exist.

After creating a bunch of contacts, here is what the paginated services open service yields:

    $ curl http://localhost:8081/contact_list -d '{}'
    {"rows":[{"id":"3d2eb82fc05f46c4907bedb1004390cd","k":["Adams","Douglas","","San Francisco"],"r":{"first_name":"Douglas","middle_name":"","last_name":"Adams","address":{"rcpt":"","fl":"42 Metaspex Way","sl":"","pc":"87501","cty":"San Francisco","reg":"CA","ctry":0}}},{"id":"ee2b839c787d4b459373f4bd029a7751","k":["Smith","Jane","","Santa Fe"],"r":{"first_name":"Jane","middle_name":"","last_name":"Smith","address":{"rcpt":"","fl":"2 Metaspex Way","sl":"","pc":"87501","cty":"Santa Fe","reg":"NM","ctry":0}}},{"id":"0f60d55ee104404ebec235f756c7d740","k":["Smith","John","","Santa Fe"],"r":{"first_name":"John","middle_name":"","last_name":"Smith","address":{"rcpt":"","fl":"1 Metaspex Way","sl":"","pc":"87501","cty":"Santa Fe","reg":"NM","ctry":0}}}]}

The contacts are returned sorted by lexicographic order of last name, first name, middle name, and city name.

If we had even more contacts, we could use `contact_list_next` and `contact_list_prev` services, which have automatically been generated, to navigate through contacts.
