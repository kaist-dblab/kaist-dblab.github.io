/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 1990-2016, KAIST                                          */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/OOSQL DB-IR-Spatial Tightly-Integrated DBMS                    */
/*    Version 5.0                                                             */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.oosql@gmail.com                                        */
/*                                                                            */
/*    Bibliography:                                                           */
/*    [1] Whang, K., Lee, J., Lee, M., Han, W., Kim, M., and Kim, J., "DB-IR  */
/*        Integration Using Tight-Coupling in the Odysseus DBMS," World Wide  */
/*        Web, Vol. 18, No. 3, pp. 491-520, May 2015.                         */
/*    [2] Whang, K., Lee, M., Lee, J., Kim, M., and Han, W., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with IR Features," In Proc. */
/*        IEEE 21st Int'l Conf. on Data Engineering (ICDE), pp. 1104-1105     */
/*        (demo), Tokyo, Japan, April 5-8, 2005. This paper received the Best */
/*        Demonstration Award.                                                */
/*    [3] Whang, K., Park, B., Han, W., and Lee, Y., "An Inverted Index       */
/*        Storage Structure Using Subindexes and Large Objects for Tight      */
/*        Coupling of Information Retrieval with Database Management          */
/*        Systems," U.S. Patent No.6,349,308 (2002) (Appl. No. 09/250,487     */
/*        (1999)).                                                            */
/*    [4] Whang, K., Lee, J., Kim, M., Lee, M., Lee, K., Han, W., and Kim,    */
/*        J., "Tightly-Coupled Spatial Database Features in the               */
/*        Odysseus/OpenGIS DBMS for High-Performance," GeoInformatica,        */
/*        Vol. 14, No. 4, pp. 425-446, Oct. 2010.                             */
/*    [5] Whang, K., Lee, J., Kim, M., Lee, M., and Lee, K., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with Spatial Database       */
/*        Features," In Proc. 23rd IEEE Int'l Conf. on Data Engineering       */
/*        (ICDE), pp. 1493-1494 (demo), Istanbul, Turkey, Apr. 16-20, 2007.   */
/*                                                                            */
/******************************************************************************/

public class OOSQL_ByteBuffer
{
    public final static int SHORT_SIZE       = 2;
    public final static int INT_SIZE         = 4;
    public final static int LONG_SIZE        = 4;
    public final static int FLOAT_SIZE       = 4;
    public final static int DOUBLE_SIZE      = 8;
    public final static int OID_SIZE         = 16;
    public final static int DATE_SIZE        = 4;
    public final static int TIME_SIZE        = SHORT_SIZE*6;
    public final static int TIMESTAMP_SIZE   = DATE_SIZE+TIME_SIZE;       

    byte[]   buffer;
    
    // constructor
    public OOSQL_ByteBuffer(int size)
    {
        buffer = new byte[size];
    }


    // return buffer
    public byte[] getBuffer()
    {
        return buffer;
    }
    
    // return size
    public int getSize()
    {
        return buffer.length;
    }
    
    public int getShortSize()
    {
        return SHORT_SIZE;
    }

    public int getIntSize()
    {
        return INT_SIZE;
    }

    public int getLongSize()
    {
        return LONG_SIZE;
    }

    public int getFloatSize()
    {
        return FLOAT_SIZE;
    }

    public int getDoubleSize()
    {
        return DOUBLE_SIZE;
    }

    public int getOIDSize()
    {
        return OID_SIZE;
    }

    public int getDateSize()
    {
        return DATE_SIZE;
    }

    public int getTimeSize()
    {
        return TIME_SIZE;
    }

    public int getTimestampSize()
    {
        return TIMESTAMP_SIZE;
    }

    
    // set byte
    public void setByte(int pos, byte value)
    {
        buffer[pos] = value;
    }
    
    // set string
    public void setString(String str)
    {
        try 
        {
            buffer = str.getBytes("US-ASCII");
        } 
        catch(java.io.UnsupportedEncodingException e)
        {
            ;
        }
    }

    // return buffer with string format
    public String toString()
    {
        int i;
        
        for (i = 0; i < buffer.length; i++)
            if (buffer[i] == 0) break;
            
        return new String(buffer, 0, i);
    } 
    
    //
    //  Note: big/little endian에 따라 code가 수정되어야 할 것 같다.
    //          현재는 UNIX (big endian)에 맞게 coding되어 있다.
    //
    
    // return buffer with short format
    public short toShort(int offset)
    {
        return (short)(((short)buffer[1+offset] & 0xff) +
                      (((short)buffer[0+offset] & 0xff) << 8));
    }        
    public short toShort()
    {
        return toShort(0);
    }


    // return buffer with int format    
    public int toInt()
    {
        return (((int)buffer[3] & 0xff)) + 
               (((int)buffer[2] & 0xff) << 8) + 
               (((int)buffer[1] & 0xff) << 16) + 
               (((int)buffer[0] & 0xff) << 24);
    }

    // return buffer with float format
    public float toFloat()
    {
        return Float.intBitsToFloat((((int)buffer[3] & 0xff)) + 
                                    (((int)buffer[2] & 0xff) << 8) + 
                                    (((int)buffer[1] & 0xff) << 16) + 
                                    (((int)buffer[0] & 0xff) << 24));
    }    
    
    // return buffer with double format    
    public double toDouble()
    {
        return Double.longBitsToDouble((((long)buffer[7] & 0xff)) + 
                                      (((long)buffer[6] & 0xff) << 8) + 
                                      (((long)buffer[5] & 0xff) << 16) + 
                                      (((long)buffer[4] & 0xff) << 24) + 
                                      (((long)buffer[3] & 0xff) << 32) + 
                                      (((long)buffer[2] & 0xff) << 40) + 
                                      (((long)buffer[1] & 0xff) << 48) + 
                                      (((long)buffer[0] & 0xff) << 56));
    }    
    
    
    public OOSQL_JavaDate toDate()
    {   
        OOSQL_JavaDate  oosqlDate = new OOSQL_JavaDate();
        
        oosqlDate.setDate((((long)buffer[3] & 0xff)) + 
                            (((long)buffer[2] & 0xff) << 8) + 
                            (((long)buffer[1] & 0xff) << 16) + 
                            (((long)buffer[0] & 0xff) << 24));
        
        return oosqlDate;
    }
        
    
    public OOSQL_JavaTime toTime()
    {
        OOSQL_JavaTime      oosqlTime = new OOSQL_JavaTime();
        
        oosqlTime.set_tzHour(toShort(SHORT_SIZE * 0));
        oosqlTime.set_tzMinute(toShort(SHORT_SIZE * 1));
        oosqlTime.set_Hour(toShort(SHORT_SIZE * 2));
        oosqlTime.set_Minute(toShort(SHORT_SIZE * 3));
        oosqlTime.set_Second(toShort(SHORT_SIZE * 4));
        oosqlTime.set_100thSec(toShort(SHORT_SIZE * 5));

        return oosqlTime;        
    }
    
    public OOSQL_JavaTimestamp toTimestamp()
    {
        OOSQL_JavaDate      oosqlDate = new OOSQL_JavaDate();
        OOSQL_JavaTime      oosqlTime = new OOSQL_JavaTime();
        OOSQL_JavaTimestamp oosqlTimestamp = new OOSQL_JavaTimestamp();
        
        oosqlDate.setDate((((long)buffer[3] & 0xff)) + 
                            (((long)buffer[2] & 0xff) << 8) + 
                            (((long)buffer[1] & 0xff) << 16) + 
                            (((long)buffer[0] & 0xff) << 24));
        
        oosqlTime.set_tzHour(toShort(SHORT_SIZE * 2));
        oosqlTime.set_tzMinute(toShort(SHORT_SIZE * 3));
        oosqlTime.set_Hour(toShort(SHORT_SIZE * 4));
        oosqlTime.set_Minute(toShort(SHORT_SIZE * 5));
        oosqlTime.set_Second(toShort(SHORT_SIZE * 6));
        oosqlTime.set_100thSec(toShort(SHORT_SIZE * 7));

        oosqlTimestamp.setD(oosqlDate);
        oosqlTimestamp.setT(oosqlTime);
        
        return oosqlTimestamp;
    }    
            
    public OID toOID()
    {
        OID     oid = new OID();
        
        oid.setDummy(buffer);
        
        return oid;
    }
}

