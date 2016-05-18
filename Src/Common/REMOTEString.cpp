/*
 * REMOTEString.h
 *
 *  Created on: 31-08-2013
 *      Author: shivaram.y
 */

#include "REMOTEString.h"
#include "CCDebugRemote.h"
/*!
 * This function compares the two strings \p dest and \p source.
 * It returns an integer less than, equal to, or greater than zero
 * if \p dest is found, to be less than, to match, or to be greater
 * than \p source, respectively.
 *
 * \param dest The first null-terminated string to compare
 * \param source The second null-terminated string to compare
 *
 * \return The function returns an integer less than, equal
 *         to, or greater than zero if \p dest is found to be less
 *         than, to match, or to be greater than \p source, respectively.
 *
 * \see Diff(const char*, const char*, unsigned int)
 */
int REMOTE::String::Diff(const char *dest, const char *source)
{
	ASSERT(dest != NULL);
	ASSERT(source != NULL);

	return strcmp(dest, source);
}

/*!
 * This function is similar to Diff(const char*, const char*),
 * except it only compares the first (at most) \p size characters
 * of \p dest and \p source.
 *
 * \param dest The first null-terminated string to compare
 * \param source The second null-terminated string to compare
 * \param size The number of characters to compare
 *
 * \return The function returns an integer less than, equal to,
 *         or greater than zero if \p dest (or the first \p size bytes
 *         thereof) is found, to be less than, to match, or to be
 *         greater than \p source, respectively.
 *
 * \see Diff(const char*, const char*)
 */
int REMOTE::String::Diff(const char *dest, const char *source, unsigned long size)
{
	ASSERT(dest != NULL);
	ASSERT(source != NULL);

	return strncmp(dest, source, size);
}


/*!
 * This function is similar to Copy(char*. const char*),
 * except that not more than \p size bytes of \p source are copied.
 * Thus, if there is no null byte among the first \p size bytes of \p source,
 * the result will not be null-terminated.
 *
 * In the case where the length of \p source is less than that of \p size,
 * the remainder of \p dest will be padded with nulls.
 *
 * \param dest (out) The destination string
 * \param source The source string
 * \param size The number of characters to copy
 *
 * \see Copy(char*. const char*), PCMem::Copy(), PCWString::Copy()
 */
void REMOTE::String::Copy(char *dest, const char *source, unsigned long size)
{
	ASSERT(dest != NULL);
	ASSERT(source != NULL);

	strncpy(dest, source, size);

	return;
}

unsigned int REMOTE::String::Length(const char *pString)
{
	return strlen(pString);
}


/*
*/

string REMOTE::String::ReplaceAll(const std::string& targetString, const std::string& src, const std::string& dest)
{
	string retString = targetString;

	string::size_type Pos = 0;
	Pos = retString.find_first_of(src, Pos);

	while (string::npos != Pos) {
		retString.replace(Pos, src.length(), dest);
		Pos += dest.length();
		Pos = retString.find_first_of(src, Pos);
	}

	return retString;
}
int REMOTE::String::Split(const string& targetString, vector<string>& tokens, const string& delimiters = " ")
{
	int numberOfSplited = 0;
	if (!tokens.empty())
		tokens.clear();

    string::size_type startPos = targetString.find_first_not_of(delimiters, 0);
    string::size_type foundPos = targetString.find_first_of(delimiters, startPos);
	if (string::npos == foundPos) {
		tokens.push_back(targetString);
		return numberOfSplited;
	}

	while (string::npos != foundPos || string::npos != startPos) {
		++numberOfSplited;
		tokens.push_back(targetString.substr(startPos, foundPos - startPos));
		startPos = targetString.find_first_not_of(delimiters, foundPos);
		foundPos = targetString.find_first_of(delimiters, startPos);
	}

	return numberOfSplited;
}
