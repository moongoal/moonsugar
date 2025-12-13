# TODO

## Standardise Constructor Failure Mode

Upon failure, each constructor shall return the approriate error code, without freeing the resources.
Only the destructor will free each resource when necessary. This implies each caller is responsible
for manually destroying objects that fail to create. This simplifies the logic by delegating to the
upper frame and leverage nested destructors to reduce the amount of code required to handle errors.
