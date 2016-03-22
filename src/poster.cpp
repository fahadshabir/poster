#include <Rcpp.h>
#include <libpostal/libpostal.h>
using namespace Rcpp;

// Consistently set up and end usage.
//[[Rcpp::export]]
void setup() {
  if (!libpostal_setup() || !libpostal_setup_language_classifier() || !libpostal_setup_parser()) {
    throw std::runtime_error("Libbpostal setup failed");
  }
}
//[[Rcpp::export]]
void end() {
  libpostal_teardown();
  libpostal_teardown_language_classifier();
  libpostal_teardown_parser();
}

//'@title Normalise postal addresses
//'@description \code{\link{normalise_addr}} takes street
//'addresses and normalises them within the context of a specific
//'country or language. It currently recognises 60 different locales.
//'
//'@param addresses a character vector of addresses to normalise
//'
//'@return a character vector of normalised addresses.
//'
//'@examples
//'# Normalise an English address!
//'normalise_addr("fourty seven love lane pinner")
//'# [1] "47 love lane pinner"
//'
//'# Normalise a French one
//'normalise_addr("Quatre-vignt-douze Ave des Champs-Élysées")
//'# "92 avenue des champs-elysees"
//'
//'@seealso \code{\link{parse_addr}} for parsing addresses.
//'@export
//[[Rcpp::export]]
CharacterVector normalise_addr(CharacterVector addresses){
  
  unsigned int input_size = addresses.size();
  CharacterVector output(input_size);
  normalize_options_t options = get_libpostal_default_options();
  size_t num_expansions;
  char **expansions;
  
  for(unsigned int i = 0; i < input_size; i++){
    
    if((i % 10000) == 0){
      Rcpp::checkUserInterrupt();
    }
    
    if(addresses[i] == NA_STRING){
      
      output[i] = NA_STRING;
      
    } else {
      
      expansions = expand_address(addresses[i], options, &num_expansions);
      if(num_expansions == 0){
        output[i] = addresses[i];
      } else {
        output[i] = std::string(expansions[0]);
      }

      expansion_array_destroy(expansions, num_expansions);
      
    }
  }
  
  
  return output;
}

String isna(std::string x){
  if(x == ""){
    return NA_STRING;
  }
  return x;
}

//'@title Parse street addresses
//'@description \code{parse_addr} parses street addresses into
//'their component parts, producing the addresses' house name,
//'number, road and suburb, city district and city, state district
//'and state, postal code, and country.
//'
//'@param addresses a character vector of addresses to parse.
//'
//'@return a data.frame of 10 columns; \code{house}, \code{house_number},
//'\code{road}, \code{suburb}, \code{city_district}, \code{city},
//'\code{state_district}, \code{state}, \code{postal_code},
//'\code{country}. Values not found in the address are represented
//'with \code{NA}s
//'
//'@examples
//'parse_addr("781 Franklin Ave Crown Heights Brooklyn NYC NY 11216 USA")
//'
//'@export
//[[Rcpp::export]]
DataFrame parse_addr(CharacterVector addresses){
  
  // Create output objects
  unsigned int input_size = addresses.size();
  CharacterVector house(input_size); // Split all of this out into something urltools style
  CharacterVector house_number(input_size);
  CharacterVector road(input_size);
  CharacterVector suburb(input_size);
  CharacterVector city_district(input_size);
  CharacterVector city(input_size);
  CharacterVector state_district(input_size);
  CharacterVector state(input_size);
  CharacterVector postal_code(input_size);
  CharacterVector country(input_size);
  address_parser_options_t options = get_libpostal_address_parser_default_options();
  
  for(unsigned int i; i < input_size; i++){
    address_parser_response_t *parsed = parse_address((char*) addresses[i], options);
    for (unsigned int n = 0; n < parsed->num_components; n++) {
      
      if((i % 10000) == 0){
        Rcpp::checkUserInterrupt();
      }
      
      // Optimise this by using an enum
      if(std::string(parsed->labels[n]) == "house"){
        house[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "house_number"){
        house_number[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "road"){
        road[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "suburb"){
        suburb[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "city_district"){
        city_district[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "city"){
        city[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "state_district"){
        state_district[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "state"){
        state[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "postal_code"){
        postal_code[i] = isna(parsed->components[n]);
      }
      
      if(std::string(parsed->labels[n]) == "country"){
        country[i] = isna(parsed->components[n]);
      }
    }
    address_parser_response_destroy(parsed);
  }
  
  return DataFrame::create(_["house"] = house,
                           _["house_number"] = house_number,
                           _["road"] = road,
                           _["suburb"] = suburb,
                           _["city_district"] = city_district,
                           _["city"] = city,
                           _["state_district"] = state_district,
                           _["state"] = state,
                           _["postal_code"] = postal_code,
                           _["country"] = country);
}