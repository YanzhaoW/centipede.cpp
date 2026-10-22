@0x85d6840a03e71113;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("centipede::writer::capnproto");

struct ValueError
{
    id @0 : UInt32;
    value @1 : Float64;
    error @2 : Float64;
}


struct Entrypoint
{
    localDerivs @0 : List(ValueError);
    globalDerivs @1 : List(ValueError);
}

struct Entry
{
    entrypoint @0 : List(Entrypoint);
}
