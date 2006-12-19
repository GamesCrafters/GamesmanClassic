#define NO_GRAPHICS //Do not include TCL files. (This should be NO_TCLTK, rather than NO_GRAPHICS, probably.)

#include "CInterface.h"
#include "../../../src/gamesman.h"

// This needs to be memoized soon
jclass getClass( JNIEnv *env, char* name ) {
  jclass theClass = (*env)->FindClass( env, name );
  return theClass;
}

jobject newPosition( JNIEnv *env, jlong value ) {
  jclass positionClass = getClass( env, "game/Position" );

  if( positionClass == NULL )
    printf( "Position class not found\n" );

  jmethodID methodID = (*env)->GetMethodID( env, positionClass, "<init>", "(J)V" );

  if( methodID == NULL )
    printf( "methodID is bad\n" );

  return (*env)->NewObject( env, positionClass, methodID, value );
}
jlong getPosition( JNIEnv *env, jobject thePosition ) {
  jclass positionClass = getClass( env, "game/Position" );
  jmethodID methodID = (*env)->GetMethodID( env, positionClass, "GetHashedPosition", "()J" );

  return (*env)->CallLongMethod( env, thePosition, methodID );
}

jobject newMove( JNIEnv *env, jobject thePosition, jint value ) {
  static jclass moveClass = NULL;
  static jmethodID methodID;
  static JNIEnv* oldenv = NULL;

  if( moveClass == NULL || oldenv != env ) {
  
    moveClass = getClass( env, "game/Move" );
    if( moveClass == NULL ) {
      printf( "Move class not found\n" );
    }

    methodID = (*env)->GetMethodID( env, moveClass, "<init>", "(Lgame/Position;I)V" );
    if( methodID == NULL ) {
      printf( "methodID is NULL\n" );
    }

    oldenv = env;
  }

  jobject obj = (*env)->NewObject( env, moveClass, methodID, thePosition, value );

  return obj;
}
jint getMove( JNIEnv *env, jobject theMove ) {
  jclass moveClass = getClass( env, "game/Move" );
  jmethodID methodID = (*env)->GetMethodID( env, moveClass, "GetHashedMove", "()I" );
  return (*env)->CallIntMethod( env, theMove, methodID );
}

jobject newValue( JNIEnv *env, VALUE value ) {
  jclass valueClass = getClass( env, "game/Value" );
  jmethodID methodID = (*env)->GetMethodID( env, valueClass, "<init>", "(I)V" );

  jfieldID fid;
  switch( value ) {
    case win:
      fid = (*env)->GetStaticFieldID( env, valueClass, "WIN", "I" );
      break;
    case lose:
      fid = (*env)->GetStaticFieldID( env, valueClass, "LOSE", "I" );
      break;
    case tie:
      fid = (*env)->GetStaticFieldID( env, valueClass, "TIE", "I" );
      break;
    case undecided:
      fid = (*env)->GetStaticFieldID( env, valueClass, "UNDECIDED", "I" );
      break;
    default: break;
  }

  jint newValue = (*env)->GetStaticIntField( env, valueClass, fid );

  return (*env)->NewObject( env, valueClass, methodID, newValue );
}

jstring newString(JNIEnv *env, const char *str)
{
  jclass Class_java_lang_String = (*env)->FindClass(env, "java/lang/String");
  jmethodID methodID_String_init = (*env)->GetMethodID(env, Class_java_lang_String,
						       "<init>", "([B)V");

  jstring result;
  jbyteArray bytes = 0;
  int len;
  if ((*env)->EnsureLocalCapacity(env, 2) < 0) {
    return NULL; /* out of memory error */
  }
  len = strlen(str);
  bytes = (*env)->NewByteArray(env, len);
  if (bytes != NULL) {
    (*env)->SetByteArrayRegion(env, bytes, 0, len,
			       (jbyte *)str);
    result = (*env)->NewObject(env, Class_java_lang_String,
			       methodID_String_init, bytes);
    (*env)->DeleteLocalRef(env, bytes);
    return result;
  } /* else fall through */
  return NULL;
}



void Java_main_CInterface_NativeInitializeGame( JNIEnv *env, jobject this )
{
  InitializeGame();
  DetermineValue( gInitialPosition );
  printf( "\n" );
  fflush( stdout );
}

jobject Java_main_CInterface_NativeDoMove( JNIEnv *env, jobject this, jobject thePosition, jint hashedMove ) {
  jlong position = getPosition( env, thePosition );

  jlong result = DoMove( position, hashedMove );

  return newPosition( env, result );
}

jobject Java_main_CInterface_NativeInitialPosition( JNIEnv *env, jobject this )
{
  jobject pos = newPosition( env, gInitialPosition );

  return pos;
}


jobject Java_main_CInterface_NativePrimitive( JNIEnv* env, jclass this, jobject thePosition )
{
  long position = getPosition( env, thePosition );

  VALUE result = Primitive( position );

  return newValue( env, result );
}

jobject Java_main_CInterface_NativeGetValueOfMove( JNIEnv* env, jclass this, jlong hashedPosition, jint hashedMove )
{
  POSITION pos;
  VALUE value;

  pos = DoMove( hashedPosition, hashedMove );
  value = GetValueOfPosition( pos );

  // TODO: Insert GoAgains
  if( value == win ) {
    value = lose;
  } else if( value == tie ) {
    value = tie;
  } else if( value == lose ) {
    value = win;
  }

  return newValue( env, value );
}

jintArray Java_main_CInterface_NativeGenerateMovesAsInts( JNIEnv* env, jclass this, jobject thePosition )
{
  int count = 0;
  long pos = getPosition( env, thePosition );

  MOVELIST* movehead = GenerateMoves( pos );
  MOVELIST* p = movehead;
  jclass moveArrayClass = (*env)->FindClass( env, "[Lgame/Move;" );

  if( moveArrayClass == NULL ) {
    printf( "Cannot find Move class\n" );
    return NULL;
  }

  while( p != NULL ) {
    ++count;
    p = p->next;
  }

  if( count == 0 ) {
    return NULL;
  }

  p = movehead;

  jint* buf = malloc( count*sizeof(jint) );

  int i;
  for( i = 0; i < count;  ++i ) {
    buf[i] = p->move;

    p = p->next;
  }

  jintArray array = (*env)->NewIntArray( env, count );
  (*env)->SetIntArrayRegion( env, array, 0, count, buf );

  return array;
}


jstring Java_main_CInterface_NativeGenericUnhash( JNIEnv* env, jclass this, jlong hashedPosition ) {
  char* str = generic_hash_unhash_tcl( hashedPosition );
  jstring jstr = newString( env, str );
  SafeFree( str );
  return jstr;
}

jstring Java_main_CInterface_NativeCustomUnhash( JNIEnv* env, jclass this, jlong hashedPosition ) {
  //ported from tkAppInit.c

  if (gCustomUnhash == NULL) {
    printf( "gCustomUnhash is NULL\n" );
    return NULL;
  }

  char *board;
  board = gCustomUnhash( hashedPosition );
  
  jstring jstr = newString( env, board );
  SafeFree(board);

  return jstr;
}



void Java_main_CInterface_NativeInitialize(JNIEnv* env, jclass this)
{
  Initialize();
}

void Java_main_CInterface_NativeInitializeDatabases(JNIEnv * env, jclass this)
{
  InitializeDatabases();
}



jobject Java_main_CInterface_NativeGetComputersMove (JNIEnv * env, jclass this, jobject thePosition) {
  return newMove( env, thePosition, GetComputersMove( getPosition(env, thePosition) ) );
}

jobject Java_main_CInterface_NativeDetermineValue(JNIEnv * env, jclass this, jobject thePosition ) {
  jobject obj;

  gMenuMode = BeforeEvaluation; /*some solvers use this for optimization*/
  obj = newValue( env, DetermineValue( getPosition(env, thePosition) ) );
  gMenuMode = Evaluated;

  return obj;
}

jint Java_main_CInterface_NativeRemoteness(JNIEnv * env, jclass this, jobject thePosition) {
  return Remoteness( getPosition( env, thePosition ) );
}

jint Java_main_CInterface_NativeMex(JNIEnv * env, jclass this, jobject thePosition){
  //return Mex( getPosition( env, thePosition ) );
  return 0;
}
