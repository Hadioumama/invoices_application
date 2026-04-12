#include "client.h"

bool Client::isValid() const {
    return !nom.isEmpty() && !prenom.isEmpty() && !email.isEmpty();
}